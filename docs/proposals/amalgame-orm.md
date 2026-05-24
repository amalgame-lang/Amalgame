# amalgame-orm — design proposal

**Status:** proposal, no code written. Target ship: v0.1 mid-2026.
This doc is a brainstorm + opinionated direction, not a final spec.
Reviewers welcome to push back on every section — the goal is an
ORM Amalgame developers will *want* to use, not "another ORM".

## Why an ORM at all

The six SQL backends shipped on 2026-05-24 (sqlite v0.3.0, mysql
v0.2.0, postgresql v0.2.0, duckdb v0.2.0, mssql v0.2.0, oracle
v0.2.0) all expose the same `Open / Exec / QueryAll / Close`
surface. That's fine for a 3-line script. It is **brutal** for a
real app:

```amalgame
// Today — raw SQLite. Welcome to 2008.
let db = SQLite.Open("./app.db")
SQLite.Exec(db, "INSERT INTO users (email, name) VALUES ('a@x', 'Alice')")
let rows = SQLite.QueryAll(db,
    "SELECT id, email, name FROM users WHERE created_at > ?")
//                                            ^^^ wait, parameter binding?
//                                                v0.2 doesn't have it yet.
//                                                String-concat the timestamp?
//                                                Hello, SQL injection.
for row in rows {
    let id = row.Get(0)
    //  ^^^ string from QueryAll. Cast to int yourself.
    let email = row.Get(1)
    //          ^^^ no idea if column 1 is email or name without checking
    //              the SQL upstream. Refactor away from positional → pain.
}
```

Every real app needs:
1. **Models that look like the data** — `user.Name`, not `row.Get(1)`.
2. **Queries that are typed end-to-end** — refactor a column rename
   and the compiler tells you what's broken.
3. **Relations** — `user.Posts()` without writing the JOIN every
   time.
4. **Migrations** — schema-as-source, diff against the live DB,
   reversible Up/Down.
5. **Backend portability** — same code targets postgres in prod and
   sqlite in tests. The 6 backends are *ready* for this; nothing
   exposes the abstraction yet.

That's an ORM. Hopefully a good one.

## What developers actually love

A sample of ORMs developers will defend in pull requests:

| ORM | Stack | Loved for |
|---|---|---|
| **Prisma** | TS/Node | schema-first DSL, generated typed client, migration diff, `prisma studio` GUI |
| **Drizzle** | TS/Node | schema in code, "less magic than Prisma", closer to SQL, edge-friendly |
| **Diesel** | Rust | compile-time SQL type-checking, `table!` macro, zero runtime cost |
| **SQLAlchemy 2.0** | Python | Core + ORM split, expression language, async, mature |
| **EF Core** | .NET | LINQ, migrations, change tracking, designer support |
| **Ecto** | Elixir | explicit, composable, schema separate from changeset, no autosave |
| **Active Record** | Ruby | convention-over-configuration, "it just works", scopes |
| **Sea-ORM** | Rust | async, derive macros, codegen entities from DB |
| **jOOQ** | Java | code-generated from DB, embedded SQL DSL |

The intersection of what they actually share — what their fans
quote when asked "why this one":

1. **Auto-complete on every column and relation.** Type a model
   variable, press `.`, see every field + every scope. No
   stringly-typed `where("emial", ...)` typos.
2. **The generated SQL is observable.** `query.ToSql()` /
   `prisma --log` / Diesel's `debug_query!` / Ecto's `to_sql/2` /
   ActiveRecord's `.to_sql`. Black-box ORMs die fast.
3. **Migrations track the schema** — write the model, the ORM
   diffs and proposes the SQL. Reversible. Repeatable across
   environments.
4. **Relations are first-class** but explicit eager/lazy. Prisma:
   `include`. Drizzle: `with`. Diesel: `belonging_to`. Ecto:
   `preload`. The bad ones (older TypeORM) auto-load relations
   transparently and produce N+1 nightmares.
5. **Transactions compose** — `db.tx(t => { ... })`, with proper
   rollback on exception, nested transaction savepoints if the
   backend supports them.
6. **Raw SQL escape hatch.** No ORM covers 100% of SQL. The good
   ones make it *trivial* to drop down: Drizzle's `sql\`…\`` tagged
   template, Diesel's `sql_query`, SQLAlchemy's `text()`.
7. **DX surface beyond queries**: REPL/playground (Prisma Studio),
   debug logging, slow query warnings, query plan integration.
8. **Test ergonomics**: fixtures, factory libraries (Factory Bot
   for AR, Test::Factory for Ecto), in-memory DB swap for tests.
9. **Performance-aware**: prepared statement caching, batch
   inserts, streaming queries for big result sets.

And what they hate (loud and often):

| Anti-pattern | Why devs scream |
|---|---|
| Magic auto-loading relations | Silent N+1, billable hours later |
| Big-ball-of-mud APIs (`.where(...).having(...).joins(...).group_by(...)` returning the same untyped builder forever) | Lost in autocomplete; the wrong call compiles but blows up at runtime |
| Schema drift | DB has `created_at`, code has `createdAt`, prod blows up at midnight |
| Async/sync split (Python SQLAlchemy pre-2.0) | Two parallel APIs to learn, lib choices bleed through stack |
| Bind parameter weakness | SQL injection, ORM dropped because security review found one place to string-concat |
| Migration auto-apply on startup with destructive ops | Dropped column in dev, restarted prod, lost data |
| Reflection-heavy runtimes (Hibernate, TypeORM) | Slow startup, weird stack traces, hard to debug |
| Per-engine quirks leaking (e.g. boolean cast on MySQL) | Code "works" until it doesn't on `JSON` columns or sequences |

## Design principles for amalgame-orm

Five non-negotiables, in priority order:

### 1. Zero runtime reflection — everything is codegen

amc compiles to C. There's no RTTI to do `getColumn("email")` at
runtime cheaply. **And that's the win.** We generate per-model
code at build time, the result reads like hand-written AM, the
compiler type-checks it, gcc inlines it. No `dlsym`, no virtual
dispatch on column access, no `void*` casts in user code.

Cost: a build step (`amc orm generate`). Same shape as Prisma's
`prisma generate`, but the output is *your* source language, not
a vendor blob.

### 2. Schema is the source of truth, and it's TOML

```toml
# schema/user.toml
[model.User]
table = "users"

[model.User.columns]
id         = { type = "i64",      pk = true, autoincrement = true }
email      = { type = "string",   not_null = true, unique = true, max_length = 255 }
name       = { type = "string",   not_null = true }
age        = { type = "i32?",     check = "age >= 0" }
created_at = { type = "datetime", default = "now()", not_null = true }
deleted_at = { type = "datetime?" }                          # soft delete marker
data       = { type = "json?" }                              # native JSON column

[model.User.relations]
posts   = { has_many = "Post",    foreign_key = "author_id", on_delete = "cascade" }
profile = { has_one  = "Profile", foreign_key = "user_id" }
roles   = { many_to_many = "Role", through = "user_roles" }

[model.User.indexes]
by_email      = { columns = ["email"], unique = true }
by_created    = { columns = ["created_at"] }
by_email_name = { columns = ["email", "name"] }              # composite

[model.User.scopes]
active        = "deleted_at IS NULL"
recent_signups = "created_at > NOW() - INTERVAL '7 days'"
```

Why TOML, not Amalgame source:
- **Tooling**: TOML is parsed by every editor, every CI, every
  GitHub UI. Code reviewers see the schema in colour even if
  they don't know AM.
- **Migrations**: schema-as-data lets us diff two versions
  cleanly (`amc orm diff v1.toml v2.toml`) and emit migration
  SQL. With AM-source schemas (Drizzle-style) we'd reparse the
  AST twice and re-do half the type system.
- **Stays in sync with `amalgame.toml`**: same format, same
  parser (`Amalgame.Formats.Toml` already in-tree), drops into
  the manifest pipeline at zero cost.

Why **not** annotations on existing AM classes (Diesel/SQLAlchemy
style):
- amc has no syntax for `@Column @Index @Unique` decorators today,
  and adding them is a parser change. TOML sidesteps that
  entirely.
- Annotations conflate "is the source of truth" between the AM
  file and the DB. Codegen-from-schema is cleaner: only one place
  to look.

### 3. The generated code is the same code you'd write by hand

```amalgame
// generated/User.am — created by `amc orm generate`. DO NOT EDIT.

namespace App.Models

import Amalgame.DateTime
import Amalgame.Json
import Amalgame.ORM

public class User {
    public id:         int
    public email:      string
    public name:       string
    public age:        int?
    public created_at: Instant
    public deleted_at: Instant?
    public data:       JsonValue?

    // Loaded-relation caches (None until `.WithPosts()` populates them).
    private _posts:   List<Post>?
    private _profile: Profile?

    // ── Lookups ────────────────────────────────────────
    public static User?  Find(db: Database, id: int)      { ... }
    public static User   FindOrThrow(db: Database, id: int) { ... }
    public static UserQuery Query()                       { ... }

    // ── Persistence ────────────────────────────────────
    public void Save(db: Database)                        { ... }
    public void Delete(db: Database)                      { ... }   // soft delete
    public void DeleteHard(db: Database)                  { ... }   // bypass soft
    public void Reload(db: Database)                      { ... }

    // ── Relations (explicit, no auto-load) ─────────────
    public List<Post> Posts(db: Database)                 { ... }
    public Profile?   GetProfile(db: Database)            { ... }
    public List<Role> Roles(db: Database)                 { ... }

    // ── Bulk helpers ───────────────────────────────────
    public static void Insert(db: Database, rows: List<User>) { ... }   // batched
    public static int  DeleteAll(db: Database, query: UserQuery) { ... }
    public static int  UpdateAll(db: Database, query: UserQuery, set: UserUpdate) { ... }
}

public class UserQuery {
    // ── Type-safe WHERE methods, generated per column ──
    public UserQuery WhereEmailEq(value: string)         { ... }
    public UserQuery WhereEmailIn(values: List<string>)  { ... }
    public UserQuery WhereEmailLike(pattern: string)     { ... }
    public UserQuery WhereAgeGt(value: int)              { ... }
    public UserQuery WhereAgeBetween(lo: int, hi: int)   { ... }
    public UserQuery WhereCreatedAfter(t: Instant)       { ... }
    public UserQuery WhereDeletedAtIsNull()              { ... }
    public UserQuery WhereDataPath(path: string, value: string)  // JSON path
                                                          { ... }

    // ── Scopes from [model.User.scopes] ────────────────
    public UserQuery Active()                            { ... }
    public UserQuery RecentSignups()                     { ... }

    // ── Eager-load relations ───────────────────────────
    public UserQuery WithPosts()                         { ... }
    public UserQuery WithProfile()                       { ... }
    public UserQuery WithPostsAndProfile()               { ... }

    // ── Order / pagination ─────────────────────────────
    public UserQuery OrderByCreatedAtDesc()              { ... }
    public UserQuery OrderByName()                       { ... }
    public UserQuery Limit(n: int)                       { ... }
    public UserQuery Offset(n: int)                      { ... }
    public UserQuery Cursor(after: int)                  { ... }   // keyset paging

    // ── Terminators ────────────────────────────────────
    public List<User> All(db: Database)                  { ... }
    public User?      First(db: Database)                { ... }
    public int        Count(db: Database)                { ... }
    public bool       Exists(db: Database)               { ... }
    public List<int>  Ids(db: Database)                  { ... }

    // ── Observability ──────────────────────────────────
    public string     ToSql()                            { ... }
    public string     Explain(db: Database)              { ... }   // EXPLAIN ANALYZE
    public UserStream Stream(db: Database, batch_size: int = 1000) { ... }
}

public class UserUpdate {
    // Generated per nullable / mutable column. Builder-pattern.
    public UserUpdate SetName(value: string)             { ... }
    public UserUpdate SetAge(value: int?)                { ... }
    public UserUpdate SetDeletedAt(value: Instant?)      { ... }
    public UserUpdate ClearDeletedAt()                   { ... }
    public UserUpdate IncrementAge(by: int)              { ... }   // atomic UPDATE … SET age = age + ?
}
```

Read that and ask: would you rather write `User.Query().WhereEmailEq("a@x").First(db)` or `db.QueryAll("SELECT … WHERE email = ?", ["a@x"]).Get(0)`? The
generated code is verbose, but **you never write it** — only
read it when debugging.

Generated `Where_<column>_*` methods compose by AND. For OR /
nested groups, the query API exposes a `Group(q => q.Or(...))`
combinator (rare enough; the common path stays clean).

### 4. Backend portability is real, not a marketing checkbox

Every query goes through a thin driver layer. The same model
code targets the six backends without `if engine == "postgres"`
branches:

```
amalgame-orm-core
   ├── Query AST (typed)
   ├── Codegen helpers
   └── Driver protocol (trait)
                  ▲
                  │ implemented by
                  │
   amalgame-orm-sqlite      → wraps amalgame-database-sqlite
   amalgame-orm-postgresql  → wraps amalgame-database-postgresql
   amalgame-orm-mysql       → wraps amalgame-database-mysql
   amalgame-orm-duckdb      → wraps amalgame-database-duckdb
   amalgame-orm-mssql       → wraps amalgame-database-mssql
   amalgame-orm-oracle      → wraps amalgame-database-oracle
```

User picks `amc package add orm-postgresql` and that's it — the
underlying `amalgame-database-postgresql` comes as a transitive
dep. The same `User` model + `User.Query()...` code is portable
because:
- Bind parameter syntax (`?` vs `$1` vs `:1`) is the driver's
  problem.
- Identifier quoting (`"name"` vs `` `name` `` vs `[name]`) is the
  driver's problem.
- Limit/offset dialect (`LIMIT ? OFFSET ?` vs `OFFSET ? ROWS
  FETCH NEXT ? ROWS ONLY`) is the driver's problem.
- `RETURNING` / `OUTPUT` / `lastInsertId()` for serial keys is the
  driver's problem.
- Boolean rendering (`TRUE` vs `1` vs `b'1'`) is the driver's
  problem.

The query AST stays SQL-flavoured enough to be predictable;
driver-specific extensions (Postgres' `INSERT ... ON CONFLICT`,
MySQL's `INSERT IGNORE`, MSSQL's `MERGE`) escape via the raw-SQL
hatch (point 7 below).

### 5. DX features that aren't optional

**N+1 detection — on by default in `--debug` builds.** If
`user.Posts(db)` is called in a loop without a parent
`.WithPosts()`, log a warning the first time + a stack trace in
debug, throw in `--strict` mode. Same idea as Active Record's
`bullet` gem but built-in.

**`ToSql()` and `Explain()` on every query.** Print the SQL +
bind parameters; run `EXPLAIN ANALYZE` and pretty-print the
plan. Make black-box behaviour impossible.

**`amc orm migrate --dry-run`.** Compute the diff between
`schema/*.toml` and the live DB's schema. Print the SQL it
*would* run. Refuse to run destructive ops (DROP COLUMN, DROP
TABLE) without `--allow-destructive`. Lock-aware: parallel CI
jobs serialise on a `_amc_migrations` table.

**`amc orm studio`.** Local-only web GUI to browse data + run
queries + inspect schema. The Prisma Studio idea, implemented on
top of `amalgame-web`. Bundled with the codegen package.

**Test fixtures.** Auto-generate `User.Mock()` with sensible
defaults from the schema:
```amalgame
let alice = User.Mock()
    .WithName("Alice")
    .Build(db)             // INSERTs + returns the saved User
let bob = User.Mock().WithEmail("b@x").Build(db)
```
And in-test transactions that auto-rollback:
```amalgame
db.TestTransaction(tx => {
    // any DB writes here are rolled back at the end
})
```

**Streaming.** `User.Query().Stream(db, batch_size: 1000)`
returns an iterator that lazy-loads batches via cursor. The big
result set never sits entirely in RAM.

**Atomic increments / decrements.** `User.Query().WhereId(42)
.Update(u => u.IncrementAge(1))` lowers to `UPDATE users SET
age = age + 1 WHERE id = 42`. No read-modify-write race.

**Soft delete is built-in.** Any column with `soft_delete = true`
gets the `Delete()` → `UPDATE ... SET deleted_at = NOW()`
shortcut and an implicit `WHERE deleted_at IS NULL` on every
query (override with `.IncludeDeleted()`).

**`Returning()` for INSERT/UPDATE.** Postgres + SQLite + MSSQL
support it natively; MySQL / MariaDB up-to-date enough do too.
Drivers that don't (older MySQL, Oracle) emulate with a second
SELECT. The API is `let saved = user.SaveReturning(db)`.

## What the user-facing code looks like

End-to-end taste of the surface:

```amalgame
namespace App

import Amalgame.ORM
import App.Models   // generated

public class Program {
    public static int Main() {
        let db = Database.Open("postgresql://app@/myapp")

        // ── Read ───────────────────────────────────────
        let alice = User.Query()
            .WhereEmailEq("alice@example.com")
            .WithPosts()                            // eager-load
            .First(db)
        if (alice == null) {
            Console.WriteLine("alice doesn't exist yet")
            return 1
        }
        for post in alice.Posts(db) {
            Console.WriteLine(post.Title)
        }

        // ── Write ──────────────────────────────────────
        let bob = new User()
        bob.email = "bob@example.com"
        bob.name = "Bob"
        bob.Save(db)
        Console.WriteLine("bob id = " + String_FromInt(bob.id))

        // ── Bulk update ────────────────────────────────
        let inactive_count = User.Query()
            .WhereCreatedBefore(Instant.Now().MinusDays(90))
            .UpdateAll(db, u => u.SetDeletedAt(Instant.Now()))
        Console.WriteLine("soft-deleted " + String_FromInt(inactive_count))

        // ── Transaction ────────────────────────────────
        db.Transaction(tx => {
            alice.name = "Alice Smith"
            alice.Save(tx)
            let role = Role.Find(tx, 1)
            alice.Roles().Attach(tx, role)
        })  // commits, or rolls back on any exception thrown inside

        // ── Raw escape hatch ───────────────────────────
        let raw = db.Raw(
            "SELECT id, name FROM users WHERE custom_func(?) = 'admin'",
            [alice.id])
        for row in raw {
            // …
        }

        // ── Observability ──────────────────────────────
        let q = User.Query().WhereEmailLike("%@example.com").Limit(10)
        Console.WriteLine(q.ToSql())
            // → SELECT id, email, name, … FROM users
            //   WHERE email LIKE $1 AND deleted_at IS NULL
            //   LIMIT 10
            //   -- $1 = '%@example.com'

        db.Close()
        return 0
    }
}
```

## Migrations

`amc orm migrate` reads `schema/*.toml` plus the migration history
table (`_amc_migrations`) and figures out what to apply. Each
migration file is generated:

```
migrations/
  2026-05-25T10-00-00_create_users.up.sql
  2026-05-25T10-00-00_create_users.down.sql
  2026-05-26T14-30-00_add_user_age.up.sql
  2026-05-26T14-30-00_add_user_age.down.sql
```

The user reviews + commits the SQL. Workflow:

```bash
# Edit schema/user.toml — add an `age` column.

amc orm diff                 # show the SQL that *would* run
amc orm generate migration   # emit the .up.sql / .down.sql pair
git add schema/ migrations/
git commit

amc orm migrate              # apply pending migrations
amc orm migrate --dry-run    # show what would apply, don't run
amc orm migrate down         # roll back the most recent
amc orm reset                # drop everything (dev only, refuses on prod via env)
```

Destructive operations require `--allow-destructive`:
- DROP COLUMN
- DROP TABLE
- ALTER COLUMN with a narrower type
- DROP INDEX (warns; rarely catastrophic but blocks queries that
  depended on it)

Migrations run inside a single transaction when the backend
supports DDL transactions (postgres, sqlite). MySQL DDL is
non-transactional → migrations either succeed completely or
require manual cleanup; the CLI warns.

## Package structure

Five packages — same dependency-light pattern as the rest of the
ecosystem:

```
amalgame-orm-core
   ├── Models: Database, Transaction, Query, Update, Stream
   ├── Codegen helpers consumed by `amc orm generate`
   ├── Migration runner (driver-agnostic)
   ├── Driver protocol (trait-like — see below)
   ├── Schema TOML parser (extends Amalgame.Formats.Toml)
   └── No backend dep

amalgame-orm-codegen
   ├── `amc orm generate` CLI subcommand (or external binary)
   ├── Reads schema/*.toml
   ├── Emits generated/<Model>.am files
   ├── Optionally regenerates on schema change (file watcher)
   └── Depends on: amalgame-orm-core, amalgame-io-filewatcher

amalgame-orm-<engine>
   ├── Implements the driver protocol for one backend
   ├── Bind parameter translation, identifier quoting, dialect quirks
   ├── Re-exports `Database.Open(...)` that resolves to the
   │   correct driver
   └── Depends on: amalgame-orm-core, amalgame-database-<engine>

amalgame-orm-testkit (optional)
   ├── In-memory SQLite shortcut for tests
   ├── Fixture builders, factory helpers
   ├── `db.TestTransaction(tx => { ... })` auto-rollback
   └── Depends on: amalgame-orm-core, amalgame-orm-sqlite
```

User dependency graph for a typical Postgres app:

```
my-app
  ├── amalgame-orm-postgresql  (provides Database.Open)
  │     ├── amalgame-orm-core
  │     └── amalgame-database-postgresql
  └── (build-time) amalgame-orm-codegen
        ├── amalgame-orm-core
        └── amalgame-io-filewatcher (for --watch)
```

The driver protocol — since AM doesn't have traits per se, we use
a "tagged dispatch" pattern (same as `amalgame-tls`'s
`TlsBackend`): every driver registers a `DriverVTable` in a
module init function, `Database.Open(url)` parses the scheme
prefix (`sqlite://`, `postgresql://`, …) and dispatches.

```amalgame
// amalgame-orm-core
public class Database {
    public static Database Open(url: string) {
        let scheme = ParseScheme(url)     // "postgresql"
        let driver = ORM.GetDriver(scheme)
        if (driver == null) {
            throw new Error("no orm driver for " + scheme + ", install amalgame-orm-" + scheme)
        }
        return new Database(driver, url)
    }
    // ...
}

// amalgame-orm-postgresql — registers itself at @c_init time
@c {
    __attribute__((constructor))
    static void register_postgresql_driver(void) {
        amalgame_orm_register_driver("postgresql", &postgresql_vtable);
    }
}
```

(The init pattern needs amc to learn `__attribute__((constructor))`
generation, or we expose a `Driver.Register()` that user code
calls explicitly. Either works; final choice is implementation
detail.)

## Implementation phases

### v0.1 — MVP, read-only, one backend
- TOML schema parsing
- Codegen for `User.am` (one model only, hand-written test
  schema)
- Query AST + builder pattern
- `WhereXxxEq` methods only (no `In`/`Like`/`Between` yet)
- Eager-load (`WithPosts`) for `has_many`
- SQLite driver (everything else stubbed)
- `ToSql()` everywhere
- `Find` / `Save` / `Delete` on the model
- `amc orm generate` CLI
- ~2 weeks

### v0.2 — write surface + 2nd backend
- INSERT / UPDATE / DELETE all generated
- Bulk operations (`UpdateAll` / `DeleteAll` / batch `Insert`)
- Postgres driver — surfaces RETURNING, JSON columns,
  array types
- Soft delete with implicit scopes
- N+1 detection (warn mode)
- ~2 weeks

### v0.3 — migrations + 3 more backends
- `amc orm diff` / `migrate` / `reset`
- Migration history table, lock semantics
- MySQL, DuckDB, MSSQL drivers
- ~1.5 weeks

### v0.4 — DX polish
- Streaming queries
- Atomic increment/decrement
- Fixture/mock helpers in `amalgame-orm-testkit`
- `amc orm studio` v1 (read-only browse)
- `--strict` N+1 mode
- ~1 week

### v0.5 — Oracle + edge cases
- Oracle driver (most painful; OCI is verbose)
- Composite primary keys
- Polymorphic associations (Active Record style — opt-in,
  documented as "use sparingly")
- Many-to-many through-tables
- ~1.5 weeks

### v1.0
- Full docs (`docs/guide/10-orm.md`)
- Backwards-compat freeze on schema TOML format
- All six backends green in CI against real services
- Example app: a small TODO web app using `amalgame-web` +
  `amalgame-orm-postgresql`

## Open questions

1. **Schema TOML or schema AM?** Pre-commit poll. TOML is the
   plan; if the editor experience suffers we revisit.

2. **Code generation: in-tree or out-of-tree?**
   - In-tree: `amc orm generate` is a subcommand of `amc`,
     `generated/` is gitignored, runs on every build.
   - Out-of-tree: `amalgame-orm-codegen` is its own binary,
     `generated/` is committed, runs on schema change only.

   I lean **out-of-tree + committed**: code review sees the
   generated diff (catches breaking changes), builds don't need
   the codegen binary, and the runtime package stays free of
   codegen complexity.

3. **Transactions vs the async stack** — `amalgame-async`
   doesn't yet thread transactions through fibers cleanly. For
   v0.1, transactions are single-thread blocking; async TX is a
   v0.6+ concern that wants the `Fiber-aware` driver protocol
   anyway.

4. **Composite & natural primary keys** — possible but adds
   significant codegen complexity. v0.1 supports single-column
   `pk` only; composite is v0.5.

5. **Schema versioning across packages** — if `amalgame-orm-core`
   v1.0 changes its codegen output shape, who regenerates? Lock
   the schema-TOML format in v0.1 and treat codegen output as
   versioned per-codegen-major like Prisma does (`@prisma/client`
   pinned to the codegen version).

6. **JSON columns ergonomics** — Postgres `jsonb` + SQLite JSON1
   + MySQL JSON each have different operators. v0.1 exposes raw
   `data: JsonValue?` + `WhereDataPath("$.field", value)` helper.
   v0.2 might add typed JSON columns with a sub-schema.

7. **Sub-query/EXISTS support** — common enough to need first-class
   syntax. `User.Query().WhereExists(Post.Query().WhereAuthorIdEq(User.Column().Id()))` is one possibility, but the
   self-referential `User.Column().Id()` is awkward. Drizzle
   uses tagged template SQL for this. v0.2 question.

## What we won't do

- **Lazy auto-loading of relations.** Explicit `.WithXxx()` is
  the rule. Hard "you forgot to load" errors in `--strict`.
- **Generated SQL in a non-SQL flavour** — no LINQ-style
  expression trees, no Hibernate-style HQL. The query API maps
  predictably to SQL, full stop.
- **Identity map / change-tracking session.** Active Record /
  EF Core / Hibernate do this; the cost is hidden complexity
  (when does my object get flushed? what about transactional
  consistency?). Models in `amalgame-orm` are dumb data;
  `.Save()` writes; the user picks the moment.
- **Auto-migration on app startup.** Some frameworks default-run
  pending migrations when the app boots. That's a great way to
  lose data. Migrations are an *operator* concern, not a *runtime*
  concern. `amc orm migrate` is explicit.
- **Stringly-typed query builders** (Hash-based `User.where(name: "Alice")`-style). Generated `WhereNameEq("Alice")` is uglier
  in source but autocompletes + refactors. The ugliness is
  worth it.
- **A REPL inside the lib.** `amc orm studio` is a separate tool,
  not embedded in the runtime.

## What I'm not sure about — push back welcome

- The `WhereXxxEq` / `WhereXxxIn` / `WhereXxxLike` explosion is
  arguably verbose. Diesel pulls this off with operator
  overloading + macros; we don't have either. Maybe `Where(c =>
  c.Email.Eq("..."))` lambdas if the closure machinery is up to
  it? But that's a heavier compile-time story.
- TOML for schema is opinionated. Drizzle proves "schema in
  code" works well in TS; could work in AM if we get
  annotation-syntax-equivalent (decorators).
- Codegen-committed-to-git is a familiar pattern (Prisma,
  buf, protoc) but doubles the diff surface. Some folks dislike
  it.
- The driver vtable pattern via `__attribute__((constructor))`
  works on POSIX gcc/clang; on Windows MSYS2 it's fine; on a
  pure MSVC build (which amc doesn't target today but might) it
  would need a different init mechanism.

If we get this right, `amc orm generate` becomes the moment an
Amalgame developer goes "oh — this is actually nicer than what I
have at work".

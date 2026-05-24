# Proposal: `Pollen` — peer-to-peer data bus + declarative workflow orchestrator

**Status:** not yet started. Captures the design of a new Amalgame
package + companion CLI for distributed LAN messaging and workflow
choreography. Direct port (then extension) of the working Node.js
prototype [`BastienMOUGET/TARMeule`](https://github.com/BastienMOUGET/TARMeule).

**Author:** v0.8.x cycle, post `amalgame-web v0.2.1` ship. Driven by the
desire to give Amalgame a **fully decentralized** messaging option
alongside the existing third-party broker clients (RabbitMQ, Kafka,
NATS, MQTT, Redis pub/sub).

**Tracking:** TBD — issues and PRs to be linked here as work begins.

**Related proposals:**
- [`amalgame-web.md`](./amalgame-web.md) — the HTTP/web sister package.
  Mosaic handles inbound HTTP, Pollen handles inter-service data flow.
- [`beyond-http.md`](./beyond-http.md) — Pollen's `Bridge` mode in v1.0
  intersects with the reverse-proxy story (a Pollen topic surfaced as
  a Mosaic WebSocket).
- [`amalgame-package-manager.md`](./amalgame-package-manager.md) — how
  the `amalgame-pollen` package is installed.

**Prerequisites:**
- `amalgame-threading` (shipped) — for the UDP acceptor + worker pool
- `amalgame-io-filewatcher` (shipped) — for hot-reload of `workflow.json`
- `amalgame-random` (shipped) — for UUIDs, ACK message IDs
- `amalgame-encoding` (shipped) — for Base64 encoding
- `amalgame-crypto` v0.2 (planned, [amalgame-web phase 2.1](./amalgame-web.md#phase-2--real-world-apps-3-4-weeks))
  — for AES-256-CBC packet encryption. **Hard blocker for v0.1.**

---

## 1. Problem

Amalgame today integrates with **five** third-party message brokers
through dedicated client packages: `rabbitmq`, `kafka`, `nats`, `mqtt`,
and `redis` (pub/sub). Every one of those requires running a broker
process (Erlang VM for RabbitMQ, JVM for Kafka, a Go binary for NATS,
a C broker for Mosquitto). Excellent for integrating into an existing
infrastructure — overkill for many LAN scenarios:

- **Industrial / IoT supervision** — capteurs → gateway → archivage,
  where every node already runs an AM binary and you don't want a
  separate broker to operate.
- **Distributed processing pipelines** on a closed network where
  latency matters and TCP handshake overhead is unwanted.
- **Symmetric topologies** where the client/server distinction doesn't
  fit (peer applications all both emit and consume).

A second gap is **workflow orchestration**. Existing orchestrators
(Temporal, Argo Workflows, Apache Airflow, n8n) are servers — they
require a central component to coordinate the pipeline. For closed
LANs where each node already knows the topology, this is again
overkill.

**TARMeule** (Node.js, ~17 KB across two files, see
[github.com/BastienMOUGET/TARMeule](https://github.com/BastienMOUGET/TARMeule))
demonstrates that both problems can be solved together with very
little code: direct UDP between nodes, with topics versioned and
subscriptions persisted as JSON files. This proposal ports it to
Amalgame and adds the workflow layer.

## 2. Goals & non-goals

### Goals (v1.0)

- Ship a **decentralized data bus** in pure Amalgame:
  - Direct UDP server ↔ server, no central broker
  - Application-level ACK + retry + UUID dedup
  - AES-256-CBC packet encryption (optional, opt-in)
  - Versioned topics with a structure schema
- Ship a **declarative workflow orchestrator** layered on top:
  - A single `workflow.json` file on a shared network mount
  - Each node identifies itself, auto-subscribes to its `consumes`
    topics, auto-routes results to its `next` nodes
  - Hot reload of the topology via filewatcher
- Wire-compatible with TARMeule v1 nodes (so a mixed AM/Node cluster
  can coexist during a migration).
- Ship a `pollen` CLI for scaffolding, dev-mode running, and topology
  inspection.

### Non-goals (deferred, possibly never)

- **Durable persistent queues** (Kafka-style replay, RabbitMQ DLQ).
  A missed message is lost beyond the ACK retry window. Use Kafka /
  RabbitMQ for that.
- **Multi-datacenter / cross-WAN messaging.** UDP rarely traverses an
  Internet firewall cleanly. Pollen is LAN or VPN-LAN.
- **Transactional sagas.** No atomic cross-topic transactions, no
  rollback semantics. Use a workflow engine (Temporal) for that.
- **Execution history.** No log of past message flows, no replay.
- **Stateful retry-with-backoff across restarts.** ACKs are
  in-memory; a node restart loses pending ACK state.
- **Multi-tenant authorization.** v1.0 has one shared symmetric key;
  per-publisher identity (Ed25519) is v1.x.

## 3. Industry survey

| Stack | What we borrow |
|---|---|
| **TARMeule** (Node.js) | The whole transport layer — direct UDP + ACK + UUID dedup + topic versioning + JSON file persistence |
| **NATS** | Subject-based addressing with wildcards (planned v0.4) |
| **ZeroMQ** | The "every app is a peer" model, no broker |
| **Argo Workflows** | The declarative `workflow.json` topology |
| **n8n / Node-RED** | Visual workflow editor (planned v0.6) |
| **MQTT** | Retained messages pattern (planned v1.x) |
| **CAN bus** | Industrial inspiration: symmetric, no master |

What we do **not** borrow:
- Broker-centric architectures (RabbitMQ, Kafka, NATS server, MQTT).
- Stateful orchestration (Temporal, Camunda).
- Pub/sub via TCP (the latency penalty is the point).

## 4. Architecture

Two layers, both decentralized:

```
┌──────────────────────────────────────────────────────────────────┐
│ Layer 2 — Workflow orchestrator                                   │
│   workflow.json on a shared network mount.                        │
│   Each node reads it, identifies its role, auto-wires topics.     │
│   Hot reload via amalgame-io-filewatcher.                         │
└──────────────────────────────────────────────────────────────────┘
                            ▲
                            │ uses
                            ▼
┌──────────────────────────────────────────────────────────────────┐
│ Layer 1 — Transport (port of TARMeule)                            │
│   UDP socket. AES-256-CBC optional. ACK + retry + UUID dedup.     │
│   Topics + subscriptions persisted as JSON files in sharedDir/.   │
│   Cross-node SYNCHRONIZATION notifications.                       │
└──────────────────────────────────────────────────────────────────┘
                            ▲
                            │ uses
                            ▼
            amalgame-threading · amalgame-crypto ·
            amalgame-io-filewatcher · amalgame-random
```

### 4.1 Layer 1 — Transport

Directly modeled on TARMeule's `UDPManager`:

- **One UDP socket per node**, bound to a configured port.
- **Cache RAM**: two maps — `topics` (UUID → { name, version,
  structure, date }) and `souscriptions` (UUID → { ip, port, topics }).
- **Persistence**: JSON files in `sharedDir/topics/<uuid>.json` and
  `sharedDir/souscriptions/<uuid>.json`. The file system is the
  source of truth; the RAM cache is rebuilt at startup.
- **Wire format**: JSON serialized payload, then optionally AES-256-CBC
  encrypted (hex-encoded). Three message `type`s:
  - `MESSAGE` — a regular publish, with `messageId`, `topic`, `data`, `timestamp`
  - `ACK` — confirms a received message by its `messageId`
  - `SYNCHRONIZATION` — notifies that a topic or souscription file
    was modified; recipients reload it
- **ACK + retry**: every `MESSAGE` is tracked in `pendingMessages`
  with a `setTimeout(ackTimeout)`. If no ACK arrives, the message is
  rebroadcast. After `maxRetries`, a `timeout` event is emitted.
- **Deduplication**: every received `messageId` is remembered for
  `ackTimeout × maxRetries × 1.5` milliseconds. Duplicate `MESSAGE`s
  are ACKed but not dispatched. Cleanup loop runs every 5 s.

### 4.2 Layer 2 — Workflow orchestrator

The new addition over TARMeule. The file `workflow.json` lives on a
network share (NFS, SMB, cluster share, anywhere all nodes can read
it). Example:

```json
{
  "name": "telemetry-pipeline",
  "version": 3,
  "nodes": {
    "acquisition-1": {
      "host": "sensor-gw-01.lan", "port": 5000,
      "emits":    ["temperature.raw"],
      "next":     ["filter-1"]
    },
    "filter-1": {
      "host": "proc-01.lan", "port": 5000,
      "consumes": ["temperature.raw"],
      "emits":    ["temperature.filtered"],
      "next":     ["aggregator-1", "archive-1"]
    },
    "aggregator-1": {
      "host": "proc-02.lan", "port": 5000,
      "consumes": ["temperature.filtered"],
      "emits":    ["temperature.minute-avg"],
      "next":     ["dashboard-1"]
    },
    "archive-1": {
      "host": "storage-01.lan", "port": 5000,
      "consumes": ["temperature.filtered"]
    },
    "dashboard-1": {
      "host": "web-01.lan", "port": 5000,
      "consumes": ["temperature.minute-avg"]
    }
  }
}
```

On startup, every Pollen node:

1. Reads `workflow.json`.
2. Looks up the node(s) whose `host:port` matches its own local
   identity (resolved via the host machine's IP/hostname or via a
   `nodeName` override in config).
3. Calls `UpsertSubscription({ ip, port, topics: consumes })` for
   each subscribed topic.
4. For each emitted topic, prepares the list of `next` nodes' IPs and
   ports as automatic recipients.
5. Watches `workflow.json` via `amalgame-io-filewatcher`. On change,
   diffs the topology and re-wires subscriptions / next-lists on the
   fly, without restarting the process.

Changing the pipeline = editing a JSON file. Adding a node = adding
an entry. Re-routing = changing `next`. No central management API.

## 5. Wire protocol

UDP datagrams. Each datagram is exactly one message. Payload is
JSON, then **conditionally** AES-256-CBC encrypted with a 16-byte
zero IV (matches TARMeule v1; v0.2 will move to a per-message random
IV prepended to the ciphertext, plus HMAC for integrity).

### 5.1 `MESSAGE`

```json
{
  "messageId": "550e8400-e29b-41d4-a716-446655440000",
  "type":      "MESSAGE",
  "topic":     { "uuid": "…", "version": 1 },
  "data":      { /* arbitrary, validated against topic schema in v0.3 */ },
  "timestamp": 1748072400123
}
```

### 5.2 `ACK`

```json
{
  "type":      "ACK",
  "messageId": "550e8400-e29b-41d4-a716-446655440000",
  "timestamp": 1748072400456
}
```

Sent in response to a `MESSAGE` from `rinfo.address:rinfo.port`.

### 5.3 `SYNCHRONIZATION`

```json
{
  "type":       "SYNCHRONIZATION",
  "entityType": "topic" | "souscription",
  "uuid":       "…",
  "timestamp":  1748072400789
}
```

Broadcast to all known peers (except self) when a topic or
subscription file is created/updated locally. Recipients reload the
corresponding file from `sharedDir/` into their RAM cache.

### 5.4 Size constraints (configurable)

| Limit | Default |
|---|---|
| Max UDP payload | 64 KB (will fragment at IP layer beyond MTU) |
| `ackTimeout` | 1000 ms |
| `maxRetries` | 5 |
| UUID memory window | `ackTimeout × maxRetries × 1.5` = 7.5 s |
| Cleanup interval | 5 s |

## 6. Public API surface

```amalgame
namespace Amalgame.Pollen

// High-level: node bound to a workflow.json role.
public class PollenNode {
    public static PollenNode New(name: string, configDir: string)

    // Handler registration. Return value: emit (with auto-route to `next`),
    // drop (no emit), or explicit forward.
    public PollenNode On(topic: string, handler: Function)
    public PollenNode OnTimeout(handler: Function)             // missed ACK
    public PollenNode OnError(handler: Function)               // crypto, parse, …

    // Manual emit (in addition to handler return).
    public void Emit(topicName: string, data: any)

    // Lifecycle.
    public void Run()                                          // UDP loop + filewatch
    public void Shutdown(timeoutMs: int)
}

// Handler return helpers.
public class Pollen {
    public static EmitResult Emit(topicName: string, data: any)
    public static EmitResult Drop()
    public static EmitResult Forward(topicName: string, data: any, to: List<string>)
}

// Low-level: raw transport without workflow.json (TARMeule-compatible).
public class PollenBus {
    public static PollenBus New(configDir: string)

    public string UpsertTopic(uuid: string?, name: string, version: int,
                              structure: Map<string,string>, description: string)
    public string UpsertSubscription(data: { ip: string, port: int, topics: List<string> })
    public void SendMessage(topicRef: { uuid: string, version: int }, data: any)
    public PollenBus OnMessage(handler: Function)
    public PollenBus OnTimeout(handler: Function)
    public void Shutdown(timeoutMs: int)
}

// Server-side configuration loaded from configDir/config.json.
public class PollenConfig {
    public string sharedDir          // path containing topics/ and souscriptions/
    public string workflowPath       // path to workflow.json (defaults to sharedDir/workflow.json)
    public int    port               // UDP listen port
    public string encryptionKey?     // 32-byte AES-256 key (hex or path to keyfile)
    public int    ackTimeoutMs       // default 1000
    public int    maxRetries         // default 5
    public string nodeName?          // override for workflow.json host matching
}
```

Example using the workflow layer:

```amalgame
namespace App
import Amalgame.Pollen

public class Program {
    public static void Main(string[] args) {
        let node = Amalgame.Pollen.PollenNode.New("filter-1", "./config")

        node.On("temperature.raw", fn(msg) {
            let v = msg.Get("value") as float
            if (v < -50.0 || v > 150.0) {
                return Amalgame.Pollen.Pollen.Drop()
            }
            return Amalgame.Pollen.Pollen.Emit("temperature.filtered", {
                value:  v,
                unit:   msg.GetString("unit"),
                source: msg.Source
            })
        })

        node.OnTimeout(fn(msgId) {
            log.Warn("ACK missed for {msgId}")
        })

        node.Run()
    }
}
```

Example using the raw layer (TARMeule-compatible):

```amalgame
let bus = PollenBus.New("./config")
let topicId = bus.UpsertTopic(null, "temperature", 1,
                              { value: "number", unit: "string" })
bus.UpsertSubscription({ ip: "192.168.1.143", port: 5000,
                         topics: ["temperature"] })
bus.SendMessage({ uuid: topicId, version: 1 },
                { value: 25, unit: "C" })
```

## 7. Configuration

`configDir/config.json`:

```json
{
  "port": 5000,
  "sharedDir": "/mnt/cluster-share/pollen",
  "workflowPath": "/mnt/cluster-share/pollen/workflow.json",
  "encryptionKey": "0123…abcd",
  "ackTimeoutMs": 1000,
  "maxRetries": 5,
  "nodeName": "filter-1"
}
```

Env vars override file (`POLLEN_PORT`, `POLLEN_SHARED_DIR`,
`POLLEN_ENCRYPTION_KEY`, …). CLI flags override env vars.

## 8. Storage layout

```
<sharedDir>/
├── workflow.json              # the orchestration file (network mount)
├── topics/
│   ├── <uuid-1>.json          # one file per topic, versioned schema
│   ├── <uuid-2>.json
│   └── …
└── souscriptions/
    ├── <uuid-1>.json          # one file per subscriber, { ip, port, topics }
    ├── <uuid-2>.json
    └── …
```

Per-file persistence (vs single sessions.json à la TARMeule) avoids
write-mutex contention when multiple nodes update topics
simultaneously. `flock()` granularity is the file, not the directory.

## 9. Concurrency model

- **Acceptor thread**: `recvfrom()` loop on the UDP socket. Each
  packet goes to a worker queue.
- **Worker pool** (built on `amalgame-threading`): N workers (default
  `2 × NumCpus`). Each pulls a packet, decrypts, parses, dispatches.
- **Timer thread**: handles ACK timeouts. Backed by a min-heap of
  `(expiry, messageId)` pairs.
- **Cleanup timer**: every 5 s, prunes expired UUIDs from the dedup
  cache.
- **Filewatcher thread**: from `amalgame-io-filewatcher`, runs the
  `workflow.json` hot-reload callback.

Per-handler state: each `On(topic, handler)` is called on a worker
thread. The handler may close over globals — those must be
thread-safe (mutex-guarded). Same rules as Mosaic's `WithState`.

## 10. Security

### v0.1 (port-level parity with TARMeule)

- Optional **AES-256-CBC** with a static 16-byte zero IV
  (TARMeule-compatible). Encrypted = JSON → AES → hex.
- A single shared symmetric key (`encryptionKey`) loaded from config.
- Failing to decrypt → drop the packet silently with an error event.
- No HMAC, no message integrity verification beyond decrypt-success.

### v0.2 (post-`amalgame-crypto` ready)

- Random per-message IV, prepended to ciphertext (12 bytes for
  GCM mode, switching from CBC).
- **HMAC** appended (or GCM auth tag) for integrity.
- Optional key rotation: nodes accept a list of valid keys, only one
  for encrypting.

### v1.x (key-based identity)

- Per-publisher **Ed25519** signature on `MESSAGE`. Public keys
  declared in `workflow.json` per node (`pubkey` field).
- ACL on topic emit: `workflow.json` may declare `allowed_publishers`
  per topic, enforced by recipients.
- Mutual auth: the dedup cache may be keyed by `(messageId, pubkey)`
  to defeat replay across compromised keys.

## 11. Observability

- **Logs** via `amalgame-logging`: ISO 8601 UTC, four levels.
  Default per-packet log: `recv ack=… src=… topic=… bytes=…`.
- **Counters** (exposed via a simple HTTP endpoint or, eventually, a
  Mosaic-served `/metrics` Prometheus):
  - `pollen_packets_received_total{type, status}`
  - `pollen_packets_sent_total{type, status}`
  - `pollen_acks_pending`
  - `pollen_dedup_cache_size`
  - `pollen_workflow_reloads_total`
  - `pollen_handler_duration_seconds_bucket{topic}`
- **Tracing**: out of scope v1.0. The `messageId` is propagable
  through handlers (`ctx.MessageId`) for cross-node log correlation.

## 12. CLI — `pollen`

```
pollen init <dir>            # scaffold a Pollen project
pollen run                   # start a node (reads ./config, ./workflow.json)
pollen send <topic> <json>   # one-shot emit from CLI (debugging)
pollen topics                # list known topics
pollen subscriptions         # list known subscriptions
pollen workflow show         # pretty-print the current workflow.json
pollen workflow validate     # check workflow.json against a JSON Schema
pollen workflow diff <file>  # compare current vs proposed
pollen version
```

Written in Amalgame, compiled by `amc`, distributed via
`amc package add pollen` (the package vendors the CLI source; install
builds it).

## 13. Roadmap

### Phase 0 — Preconditions

| # | Item | Status |
|---|---|---|
| 0.1 | `amalgame-crypto` v0.2 with AES-256-CBC | tracked in [amalgame-web phase 2.1](./amalgame-web.md#phase-2--real-world-apps-3-4-weeks) |
| 0.2 | TARMeule v1 frozen as wire reference | done — repo at `BastienMOUGET/TARMeule` |

### Phase 1 — Transport port (~1-2 weeks)

| # | Item | Effort |
|---|---|---|
| 1.1 | `amalgame-pollen` package skeleton (manifest, layout) | 0.5 d |
| 1.2 | UDP socket binding via `@c {}` (Berkeley sockets / WinSock) | 1 d |
| 1.3 | JSON serialize/deserialize for MESSAGE/ACK/SYNCHRONIZATION | 0.5 d |
| 1.4 | AES-256-CBC encrypt/decrypt (depends 0.1) | 0.5 d |
| 1.5 | ACK + retry + timeout machinery, UUID dedup cache | 2 d |
| 1.6 | Topic + Subscription persistence (JSON files in sharedDir/) | 1 d |
| 1.7 | RAM cache, load-at-startup, reload-on-SYNC | 1 d |
| 1.8 | `PollenBus` public API (`Upsert*`, `SendMessage`, `OnMessage`) | 1 d |
| 1.9 | Interop test against a TARMeule v1 Node.js node | 1 d |
| 1.10 | `pollen` CLI: `init`, `send`, `topics`, `subscriptions` | 1 d |

### Phase 2 — Workflow layer (~1-2 weeks)

| # | Item | Effort |
|---|---|---|
| 2.1 | `workflow.json` schema + JSON Schema for `pollen workflow validate` | 1 d |
| 2.2 | Node-self-identification (`host:port` match, `nodeName` override) | 0.5 d |
| 2.3 | Auto-subscription from `consumes` | 1 d |
| 2.4 | Auto-routing to `next` (handler return → emit to nexts) | 1 d |
| 2.5 | `PollenNode` API (`On(topic)`, `Pollen.Emit/Drop/Forward`) | 1 d |
| 2.6 | Filewatcher integration, hot reload of topology | 1 d |
| 2.7 | `pollen workflow show` / `diff` | 0.5 d |

### Phase 3 — Robustness (~1 week)

| # | Item | Effort |
|---|---|---|
| 3.1 | Schema validation on emit AND receive against topic `structure` | 1 d |
| 3.2 | QoS per topic / per edge (fire-and-forget, ack-required, at-least-once) | 1 d |
| 3.3 | Multicast discovery (optional, opt-in) for dynamic nodes | 1 d |
| 3.4 | Graceful shutdown (drain pending ACKs, SIGTERM handling) | 0.5 d |
| 3.5 | Worker pool tuning, backpressure when queue full | 1 d |
| 3.6 | Per-message IV (move from CBC zero-IV to GCM with random IV) | 1 d |

### Phase 4 — UX + ops (~1-2 weeks)

| # | Item | Effort |
|---|---|---|
| 4.1 | Structured logs via `amalgame-logging` | 0.5 d |
| 4.2 | `/metrics` endpoint (Prometheus exposition) | 1 d |
| 4.3 | `pollen.toml` config + env-var override + CLI flags | 0.5 d |
| 4.4 | `amalgame-service` integration (systemd + Windows SCM) | 0.5 d |
| 4.5 | Visual workflow editor — small Mosaic app, drag-and-drop | 3-4 d |
| 4.6 | Bridge to Mosaic: Pollen topic ↔ WebSocket | 1-2 d |
| 4.7 | Bridge to MQTT / RabbitMQ (optional, edge nodes) | 2 d |

### Phase 5 — Identity + auth (~1 week)

| # | Item | Effort |
|---|---|---|
| 5.1 | Ed25519 per-node keypair, public keys in `workflow.json` | 1 d |
| 5.2 | Message signing on emit, verification on receive | 1 d |
| 5.3 | Topic-level ACL (`allowed_publishers`) | 1 d |
| 5.4 | Key rotation procedure (docs + tooling) | 0.5 d |

**Total estimate**: ~7-9 weeks of focused work to reach the v1.0 line
(end of phase 4). Phase 5 (Ed25519 identity) is v1.x.

## 14. External dependencies

| Dep | Reason | Resolved via |
|---|---|---|
| `amalgame-threading` | UDP acceptor + worker pool + timer | already in ecosystem |
| `amalgame-io-filewatcher` | hot-reload `workflow.json` | already in ecosystem |
| `amalgame-crypto` (≥ v0.2) | AES-256 + Ed25519 | tracked in [amalgame-web phase 2.1](./amalgame-web.md) |
| `amalgame-random` | UUID generation, CSPRNG for IVs | already in ecosystem |
| `amalgame-encoding` | Base64, hex | already in ecosystem |
| `amalgame-logging` | structured logs | already in ecosystem |
| `amalgame-service` | systemd / Windows SCM packaging | already in ecosystem |

No external C library required. The UDP socket layer is BSD sockets
/ WinSock via a thin `@c {}` binding, similar to what `amalgame-net-http`
does for TCP.

## 15. Naming

| Layer | Name |
|---|---|
| amc package | `amalgame-pollen` |
| AM namespace | `Amalgame.Pollen` |
| CLI binary | `pollen` |
| Repository slug | `amalgame-lang/amalgame-pollen` |
| Marketing | "Pollen — Amalgame's peer-to-peer bus + workflow orchestrator" |

Rationale for "Pollen": pollen travels without a central coordinator,
carried from flower to flower by autonomous carriers, transporting
the most valuable thing to propagate (genetic information). The
metaphor matches the architecture: autonomous nodes that emit and
receive directly, on a network where coordination lives in the shared
data (the `workflow.json`), not in a conductor.

The first prototype repo was named "TARMeule" by its author; we keep
that name attached to the Node.js reference implementation for
historical continuity, and use "Pollen" for the Amalgame port.

## 16. Decisions log

1. **Port TARMeule rather than reinvent.** TARMeule's wire protocol
   already works; building a new protocol from scratch adds risk
   and breaks interop with the existing Node.js prototype.
2. **UDP, not TCP.** Latency is the priority; the LAN target makes
   per-packet reliability via app-level ACK acceptable.
3. **JSON wire format** (not MessagePack/CBOR). Human-debuggable.
   MessagePack may be revisited in v1.x if payload size matters.
4. **JSON Schema for `workflow.json`** rather than a custom DSL.
   Editable in any text editor or web tool, validable with standard
   tooling.
5. **Workflow file on a network share** rather than a gossip protocol.
   Simpler; one file as source of truth; manual conflict resolution.
   Distributed consensus (Raft, etc.) is overkill for a few nodes.
6. **One file per topic / per subscription** rather than one
   `topics.json` / one `souscriptions.json`. Avoids global write-mutex
   contention. Each file has its own `flock()`.
7. **Cache RAM with file persistence**, not pure file-based reads.
   Per-message file I/O would dominate latency. RAM cache + cross-node
   sync notifications keep both consistency and performance.
8. **No execution history / no replay.** Out of scope. Use Kafka if
   you need replay. Pollen optimizes for the LAN-real-time case.
9. **`PollenNode` (workflow-aware) and `PollenBus` (raw)** as two
   separate APIs rather than one with a flag. Different mental models;
   keeping them distinct avoids API drift.
10. **Symmetric key (AES) in v0.1, asymmetric (Ed25519) in v1.x.**
    Ships value early; identity layer is additive.
11. **Wire-compatible with TARMeule v1.** Allows mixed-language
    clusters during migration. May diverge in v0.2 (random IV)
    flagged via a config option.
12. **Direct Mosaic bridge as a v1.0 feature, not via a separate
    package.** Mosaic and Pollen are sister projects, so a built-in
    bridge is justified rather than a third-party glue lib.
13. **No multi-datacenter ambition.** UDP across the Internet is
    a losing battle. Pollen explicitly targets LAN / VPN-LAN.
14. **Pure-AM implementation, no third-party C lib.** Unlike Mosaic
    which leans on OpenSSL + nghttp2, Pollen's wire complexity is low
    enough that a pure-AM impl is feasible and avoids a dependency
    surface.

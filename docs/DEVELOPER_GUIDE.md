# Amalgame — Developer Guide

> **Version** : 0.3.0
> **Extension** : `.am`
> **Compiler** : `amc`
> **License** : Apache 2.0

---

## Table of Contents

1. [Introduction](#1-introduction)
2. [Installation & Build](#2-installation--build)
3. [Hello, World!](#3-hello-world)
4. [Program Structure](#4-program-structure)
5. [Types](#5-types)
6. [Variables](#6-variables)
7. [String Interpolation](#7-string-interpolation)
8. [Operators & Precedence](#8-operators--precedence)
9. [Control Flow](#9-control-flow)
10. [Functions & Methods](#10-functions--methods)
11. [Classes](#11-classes)
12. [Inheritance](#12-inheritance)
13. [Records & Data Classes](#13-records--data-classes)
14. [Interfaces & Traits](#14-interfaces--traits)
15. [Enums](#15-enums)
16. [Pattern Matching](#16-pattern-matching)
17. [Generics](#17-generics)
18. [Lambdas & Closures](#18-lambdas--closures)
19. [Null Safety](#19-null-safety)
20. [Async / Await](#20-async--await)
21. [Error Handling](#21-error-handling)
22. [Collections](#22-collections)
23. [Decorators](#23-decorators)
24. [Memory Management](#24-memory-management)
25. [Modules & Imports](#25-modules--imports)
26. [Compiler Pipeline](#26-compiler-pipeline)
27. [Error Messages](#27-error-messages)
28. [Current Limitations](#28-current-limitations)

---

## 1. Introduction

Amalgame is a modern, statically typed programming language that transpiles to C.
It distills the best features from today's most productive languages:

| Feature | Inspired by |
|---|---|
| Type inference | Kotlin / Swift |
| Null safety | Kotlin / Swift |
| Pattern matching | Rust / Haskell |
| Result / Option | Rust |
| Data classes | Kotlin |
| Extension methods | Kotlin / C# |
| Pipeline `\|>` | F# / Elixir |
| Async / Await | C# / JavaScript |
| GC by default | Go / C# |
| Memory decorators | Nim / D |

The philosophy is simple: **you should never have to choose between safety, expressiveness, and performance**.

---

## 2. Installation & Build

### Prerequisites

```bash
sudo apt install valac libglib2.0-dev libgee-0.8-dev \
                 meson ninja-build libgc-dev gcc
```

### Build

```bash
git clone https://github.com/BastienMOUGET/Amalgame
cd Amalgame
meson setup build
cd build && ninja
```

### Verify

```bash
./amc --version
# Amalgame Transpiler v0.3.0
```

### Usage

```bash
# Compile and run an Amalgame file
amc hello.am
./hello

# Specify C output file
amc hello.am -o out.c

# Show AST (debug)
AMC_DEBUG=1 amc hello.am

# Skip type checking
amc hello.am --no-typecheck
```

---

## 3. Hello, World!

```amalgame
namespace MyApp

import Amalgame.IO

public class Program {
    public static void Main(string[] args) {
        let name = "World"
        Console.WriteLine("Hello {name}!")
    }
}
```

Save as `hello.am`, then:

```bash
amc hello.am
./hello
# Hello World!
```

---

## 4. Program Structure

Every Amalgame file follows this structure:

```amalgame
namespace MyApp           // 1. Namespace (required)

import Amalgame.IO        // 2. Imports (optional)
import Amalgame.Net

public class MyClass { …  // 3. Top-level declarations
}

public record Point(…)
public enum Role { … }
```

### Namespace

```amalgame
namespace MyApp
namespace MyApp.Models
namespace MyApp.Services.Auth
```

### Imports

```amalgame
import Amalgame.IO
import Amalgame.Net
import Amalgame.Collections
import MyApp.Models as Models   // aliased import
```

---

## 5. Types

### Primitive types

| Type | Description | Example |
|---|---|---|
| `int` | 64-bit signed integer | `42`, `-7`, `1_000_000` |
| `float` | 32-bit float | `3.14`, `-0.5` |
| `double` | 64-bit float | `3.14159265` |
| `bool` | Boolean | `true`, `false` |
| `string` | UTF-8 string | `"hello"` |
| `char` | Single character | `'a'` |
| `byte` | Unsigned 8-bit | `0xFF` |
| `void` | No value | *(return type only)* |

### Sized integers

```amalgame
let a: i8  = 127
let b: i16 = 32767
let c: i32 = 2_147_483_647
let d: i64 = 9_223_372_036_854_775_807
let e: u8  = 255
let f: u16 = 65535
let g: u32 = 4_294_967_295
let h: u64 = 18_446_744_073_709_551_615
let i: f32 = 3.14
let j: f64 = 3.141592653589793
```

### Nullable types

Any type can be made nullable by appending `?`:

```amalgame
let name: string?  = null
let age:  int?     = null
let p:    Player?  = null
```

### Generic types

```amalgame
let names:   List<string>
let scores:  Map<string, int>
let result:  Result<Player>
let maybe:   Option<int>
let task:    Task<string>
let fn:      Func<int, int, bool>
```

### Tuple types

```amalgame
let pair:   (int, string)    = (42, "hello")
let triple: (int, int, int)  = (1, 2, 3)
```

### Array types

```amalgame
let args: string[]
let data: int[]
```

---

## 6. Variables

### Immutable (`let`)

```amalgame
let x = 42              // inferred as int
let name = "Arthus"     // inferred as string
let pi: float = 3.14    // explicit type
```

`let` variables cannot be reassigned:

```amalgame
let x = 10
x = 20   // ❌ error: cannot assign to immutable binding 'x'
```

### Mutable (`var`)

```amalgame
var count = 0
count = count + 1   // ✅

var name: string = "Arthus"
name = "Merlin"     // ✅
```

### Memory decorator (optional)

```amalgame
@arc   let player = new Player("Arthus")   // reference-counted
@stack let point  = new Point(3.0, 4.0)   // stack allocated
@arena let buf    = new Buffer(1024)       // arena allocated
```

---

## 7. String Interpolation

Amalgame supports inline expression interpolation inside strings using `{expr}`:

```amalgame
let name  = "Arthus"
let level = 42
let hp    = 85.5

Console.WriteLine("Player: {name}")
Console.WriteLine("Level {level}, HP: {hp}")
Console.WriteLine("{name} is level {level} with {hp} HP")
```

Any expression can appear inside `{}`:

```amalgame
Console.WriteLine("Sum: {1 + 2}")
Console.WriteLine("Upper: {name}")
Console.WriteLine("Alive: {player.Health > 0}")
```

Multi-line strings use triple quotes:

```amalgame
let text = """
    Hello {name},
    Welcome to Amalgame!
"""
```

---

## 8. Operators & Precedence

### Precedence (lowest to highest)

| Priority | Operator(s) | Description |
|---|---|---|
| 1 | `=` `+=` `-=` `*=` `/=` | Assignment |
| 2 | `\|>` | Pipeline |
| 3 | `??` | Null coalescing |
| 4 | `\|\|` | Logical OR |
| 5 | `&&` | Logical AND |
| 6 | `==` `!=` | Equality |
| 7 | `<` `>` `<=` `>=` `is` | Comparison / type check |
| 8 | `..` | Range |
| 9 | `+` `-` | Addition |
| 10 | `*` `/` `%` | Multiplication |
| 11 | `!` `-` (unary) | Unary |
| 12 | `^` | Power |
| 13 | `.` `?.` `[]` `()` | Member access, call |
| 14 | literals, identifiers | Primary |

### Arithmetic

```amalgame
let sum  = 10 + 3    // 13
let diff = 10 - 3    // 7
let prod = 10 * 3    // 30
let quot = 10 / 3    // 3
let rem  = 10 % 3    // 1
let pow  = 2 ^ 10    // 1024
```

### Comparison & logical

```amalgame
let gt  = 5 > 3      // true
let eq  = 5 == 5     // true
let ne  = 5 != 3     // true
let and = true && false   // false
let or  = true || false   // true
let not = !true           // false
```

### Range

```amalgame
let r = 1..10    // range from 1 to 10 (inclusive)
```

### Pipeline `|>`

```amalgame
let result = players
    |> Where(p => p.Level >= 35)
    |> OrderBy(p => p.Level)
    |> ToList()
```

Equivalent to:
```amalgame
let result = ToList(OrderBy(Where(players, p => p.Level >= 35), p => p.Level))
```

### Null coalescing `??`

```amalgame
let name: string? = null
let display = name ?? "Anonymous"   // "Anonymous"
```

### Null-safe member access `?.`

```amalgame
let length = player?.Name?.Length ?? 0
```

### String concatenation `+`

```amalgame
let greeting = "Hello" + ", " + "World!"
```

---

## 9. Control Flow

### `if` / `else if` / `else`

```amalgame
if (score >= 90) {
    Console.WriteLine("A")
} else if (score >= 80) {
    Console.WriteLine("B")
} else if (score >= 70) {
    Console.WriteLine("C")
} else {
    Console.WriteLine("F")
}
```

### `while`

```amalgame
var i = 0
while (i < 10) {
    Console.WriteLine("i = {i}")
    i = i + 1
}
```

### `for`

```amalgame
for (var i = 0; i < 10; i = i + 1) {
    Console.WriteLine("i = {i}")
}
```

### `foreach`

```amalgame
foreach (let player in players) {
    Console.WriteLine(player.Name)
}

foreach (var score in scores) {
    Console.WriteLine("Score: {score}")
}
```

### `break` / `continue`

```amalgame
var i = 0
while (i < 100) {
    if (i == 5) { break }
    if (i % 2 == 0) { i = i + 1; continue }
    Console.WriteLine(i)
    i = i + 1
}
```

### `guard`

Guard exits the current scope early when the condition is false:

```amalgame
public void ProcessPlayer(Player? p) {
    guard p != null else {
        Console.WriteLine("No player")
        return
    }
    // p is non-null here
    Console.WriteLine(p.Name)
}
```

### `try` / `catch`

```amalgame
try {
    let data = File.ReadAll("config.json")
    Console.WriteLine(data)
} catch (IOException e) {
    Console.WriteLine("Error: {e.Message}")
}
```

---

## 10. Functions & Methods

### Method declaration

```amalgame
public ReturnType MethodName(Type param1, Type param2) {
    // body
    return value
}
```

### Expression body (single expression)

```amalgame
public int Double(int x) => x * 2

public bool IsAdult(int age) => age >= 18

public string Greet(string name) => "Hello {name}!"
```

### Static methods

```amalgame
public class MathHelper {
    public static int Add(int a, int b) {
        return a + b
    }

    public static int Max(int a, int b) => a > b ? a : b
}

let sum = MathHelper.Add(21, 21)   // 42
```

### Default parameters

```amalgame
public void Log(string message, string level = "INFO") {
    Console.WriteLine("[{level}] {message}")
}

Log("Server started")             // [INFO] Server started
Log("Disk full", "ERROR")        // [ERROR] Disk full
```

### Named arguments

```amalgame
public class Player {
    public Player(string name, int health = 100, int level = 1) { … }
}

let p = new Player("Arthus", level: 42, health: 80)
```

### Generic methods

```amalgame
public T First<T>(List<T> items) {
    return items[0]
}
```

### Pure functions

Pure functions have no side effects and always return the same output for the same input:

```amalgame
public pure int Square(int x) => x * x
```

---

## 11. Classes

### Basic class

```amalgame
public class Animal {
    public Name: string
    public Age:  int

    public Animal(string name, int age) {
        this.Name = name
        this.Age  = age
    }

    public void Speak() {
        Console.WriteLine("{this.Name} makes a sound!")
    }

    public string Describe() {
        return "{this.Name} is {this.Age} years old"
    }
}

let cat = new Animal("Cat", 3)
cat.Speak()
Console.WriteLine(cat.Describe())
```

### Field declaration syntax

Fields use `Name: Type` syntax (Kotlin-style):

```amalgame
public class Player {
    public  Name:   string     // public field
    private Health: int        // private field
    public  Level:  int = 1   // with default value
}
```

### Properties with getters/setters

```amalgame
public class Circle {
    private Radius: float

    public Area: float {
        get => 3.14159 * this.Radius * this.Radius
    }

    public Diameter: float {
        get => this.Radius * 2
        set => this.Radius = value / 2
    }
}
```

### Access modifiers

```amalgame
public class MyClass {
    public    PublicField:    int   // accessible everywhere
    private   PrivateField:   int   // only within this class
    protected ProtectedField: int   // this class and subclasses
    internal  InternalField:  int   // same namespace (default)
}
```

### Static members

```amalgame
public class Counter {
    private static Count: int = 0

    public static void Increment() {
        Counter.Count = Counter.Count + 1
    }

    public static int GetCount() => Counter.Count
}

Counter.Increment()
Counter.Increment()
Console.WriteLine(Counter.GetCount())   // 2
```

### `this`

```amalgame
public class Rectangle {
    public Width:  float
    public Height: float

    public Rectangle(float width, float height) {
        this.Width  = width
        this.Height = height
    }

    public float Area()      => this.Width * this.Height
    public float Perimeter() => 2 * (this.Width + this.Height)
}
```

---

## 12. Inheritance

### `extends`

```amalgame
public class Shape {
    public Name: string

    public Shape(string name) {
        this.Name = name
    }

    public float Area() {
        return 0.0
    }
}

public class Circle extends Shape {
    public Radius: float

    public Circle(string name, float radius) {
        this.Name   = name      // access parent field
        this.Radius = radius
    }

    public float Area() {
        return 3.14159 * this.Radius * this.Radius
    }
}

public class Rectangle extends Shape {
    public Width:  float
    public Height: float

    public Rectangle(string name, float w, float h) {
        this.Name   = name
        this.Width  = w
        this.Height = h
    }

    public float Area() {
        return this.Width * this.Height
    }
}

let c = new Circle("Sun", 5.0)
Console.WriteLine("Area: {c.Area()}")       // Area: 78.53975
Console.WriteLine("Name: {c.Name}")         // Name: Sun
```

### `implements` (interfaces)

```amalgame
public class Player implements IDamageable, IPrintable {
    // must implement all interface methods
}
```

### Method overriding

```amalgame
public class Animal {
    public virtual string Sound() => "..."
}

public class Dog extends Animal {
    public override string Sound() => "Woof!"
}

public class Cat extends Animal {
    public override string Sound() => "Meow!"
}
```

---

## 13. Records & Data Classes

### Record

Records are immutable value types with auto-generated constructors:

```amalgame
public record Point(float X, float Y)

public record Color(int R, int G, int B)

let p     = new Point(3.0, 4.0)
let red   = new Color(255, 0, 0)

Console.WriteLine("Point: ({p.X}, {p.Y})")   // Point: (3, 4)
```

Records can have methods:

```amalgame
public record Point(float X, float Y) {
    public float Length() {
        return this.X * this.X + this.Y * this.Y
    }

    public string ToString() {
        return "({this.X}, {this.Y})"
    }
}
```

### Data class

Data classes are like records but support mutability:

```amalgame
public data class Player(
    string Name,
    int    Health = 100,
    int    Level  = 1
)

let hero   = new Player("Arthus", 85, 42)
let wizard = new Player("Merlin")     // Health=100, Level=1

Console.WriteLine("{hero.Name} HP:{hero.Health} Lvl:{hero.Level}")
```

### `with` expressions (non-destructive update)

```amalgame
let hero    = new Player("Arthus", 100, 42)
let wounded = hero with { Health = 60 }
let leveled = hero with { Level = 43, Health = 100 }

// hero is unchanged
Console.WriteLine(hero.Health)     // 100
Console.WriteLine(wounded.Health)  // 60
```

---

## 14. Interfaces & Traits

### Interface

```amalgame
public interface IDamageable {
    TakeDamage(int amount) -> void
    Heal(int amount)       -> void
    IsAlive()              -> bool
}

public interface IPrintable {
    ToString() -> string
}
```

Interface with default implementation:

```amalgame
public interface ILoggable {
    Log(string message) -> void default {
        Console.WriteLine("[LOG] {message}")
    }
}
```

### Implementing interfaces

```amalgame
public class Player implements IDamageable {
    public Health: int

    public void TakeDamage(int amount) {
        this.Health = this.Health - amount
    }

    public void Heal(int amount) {
        this.Health = this.Health + amount
    }

    public bool IsAlive() => this.Health > 0
}
```

### Trait

Traits are like interfaces but can carry state and full implementations:

```amalgame
public trait Printable {
    public string ToString() {
        return "Object"
    }

    public void Print() {
        Console.WriteLine(this.ToString())
    }
}

public class Player implements Printable {
    public Name: string

    public string ToString() {
        return "Player({this.Name})"
    }
}
```

---

## 15. Enums

### Basic enum

```amalgame
public enum Direction {
    North, South, East, West
}

let dir = Direction.North
Console.WriteLine(dir)
```

### Enum with values

```amalgame
public enum Role {
    Tank,
    Healer,
    DPS,
    Support
}
```

### Rich enum (with data)

```amalgame
public enum Shape {
    Circle(float),
    Rectangle(float, float),
    Triangle(float, float, float)
}
```

### Enum methods

```amalgame
public enum Season {
    Spring, Summer, Autumn, Winter

    public bool IsWarm() {
        match this {
            Season.Spring => true,
            Season.Summer => true,
            _             => false
        }
    }
}

let s = Season.Summer
Console.WriteLine(s.IsWarm())   // true
```

---

## 16. Pattern Matching

### `match` on values

```amalgame
let health = 75

match health {
    100        => Console.WriteLine("Full health"),
    75..99     => Console.WriteLine("Slightly wounded"),
    50..74     => Console.WriteLine("Wounded"),
    1..49      => Console.WriteLine("Critical"),
    0          => Console.WriteLine("Dead"),
    _          => Console.WriteLine("Unknown")
}
```

### `match` on enums

```amalgame
match player.Role {
    Role.Tank    => Console.WriteLine("Tanking!"),
    Role.Healer  => Console.WriteLine("Healing!"),
    Role.DPS     => Console.WriteLine("Dealing damage!"),
    _            => Console.WriteLine("Supporting!")
}
```

### `match` on types

```amalgame
match shape {
    Circle c      => Console.WriteLine("Circle r={c.Radius}"),
    Rectangle r   => Console.WriteLine("Rect {r.Width}x{r.Height}"),
    _             => Console.WriteLine("Unknown shape")
}
```

### Guard clauses

```amalgame
match player {
    Player p if p.Level >= 50  => Console.WriteLine("Veteran: {p.Name}"),
    Player p if p.Level >= 10  => Console.WriteLine("Experienced: {p.Name}"),
    Player p                   => Console.WriteLine("Rookie: {p.Name}")
}
```

### Destructuring

```amalgame
match point {
    Point(0.0, 0.0)  => Console.WriteLine("Origin"),
    Point(x, 0.0)    => Console.WriteLine("On X axis at {x}"),
    Point(0.0, y)    => Console.WriteLine("On Y axis at {y}"),
    Point(x, y)      => Console.WriteLine("At ({x}, {y})")
}
```

### `match` as expression

```amalgame
let grade = match score {
    90..100 => "A",
    80..89  => "B",
    70..79  => "C",
    _       => "F"
}
```

---

## 17. Generics

### Generic classes

```amalgame
public class Box<T> {
    public Value: T

    public Box(T value) {
        this.Value = value
    }

    public T Get() => this.Value
}

let intBox    = new Box<int>(42)
let strBox    = new Box<string>("hello")

Console.WriteLine(intBox.Get())   // 42
Console.WriteLine(strBox.Get())   // hello
```

### Constrained generics

```amalgame
public class SortedList<T : Comparable> {
    // T must implement Comparable
}
```

### Generic methods

```amalgame
public T Max<T : Comparable>(T a, T b) => a > b ? a : b

let biggest = Max<int>(10, 42)    // 42
let longest = Max<string>("hi", "hello")   // "hi" (by length)
```

### Built-in generic types

```amalgame
let list:    List<Player>
let map:     Map<string, int>
let set:     Set<string>
let result:  Result<Player>
let option:  Option<int>
let task:    Task<string>
```

---

## 18. Lambdas & Closures

### Lambda syntax

Single parameter:
```amalgame
p => p.Level >= 35
```

Multiple parameters:
```amalgame
(a, b) => a + b
```

Block body:
```amalgame
(x, y) => {
    let sum = x + y
    return sum * 2
}
```

### Lambdas in practice

```amalgame
let players = new List<Player>(…)

// Filter
let veterans = players |> Where(p => p.Level >= 35)

// Transform
let names = players |> Select(p => p.Name)

// Action
players |> ForEach(p => Console.WriteLine(p.Name))
```

### Stored lambdas

```amalgame
let double:    Func<int, int>        = x => x * 2
let add:       Func<int, int, int>   = (a, b) => a + b
let greet:     Func<string, string>  = name => "Hello {name}!"
let printLine: Action<string>        = s => Console.WriteLine(s)
```

### Higher-order functions

```amalgame
public List<T> Filter<T>(List<T> items, Func<T, bool> predicate) {
    var result = new List<T>()
    foreach (let item in items) {
        if (predicate(item)) {
            result.Add(item)
        }
    }
    return result
}

let adults = Filter(people, p => p.Age >= 18)
```

### Closures

Lambdas capture variables from their enclosing scope:

```amalgame
let threshold = 50

let highScorers = players |> Where(p => p.Score >= threshold)
// threshold is captured from the outer scope
```

---

## 19. Null Safety

Amalgame eliminates null pointer exceptions through the type system.

### Non-nullable by default

```amalgame
let name: string = "Arthus"   // cannot be null
name = null                    // ❌ compile error
```

### Nullable types

```amalgame
let name: string? = null       // explicitly nullable
let age:  int?    = 42
```

### Null check

```amalgame
let player: Player? = FindPlayer("Arthus")

if (player != null) {
    Console.WriteLine(player.Name)   // safe — player is non-null here
}
```

### Null-safe access `?.`

```amalgame
let len = player?.Name?.Length   // returns int? — null if player is null
```

### Null coalescing `??`

```amalgame
let name = player?.Name ?? "Unknown"   // "Unknown" if player is null
let hp   = player?.Health ?? 0
```

### Guard for null

```amalgame
public void Process(Player? player) {
    guard player != null else {
        return
    }
    // player is non-null from here
    Console.WriteLine(player.Name)
}
```

---

## 20. Async / Await

### Async methods

```amalgame
public async Task<string> FetchUser(string id) {
    let response = await Http.GetAsync("https://api.example.com/users/{id}")
    return response.Body
}
```

### `await`

```amalgame
public async Task Main() {
    let user = await FetchUser("123")
    Console.WriteLine("User: {user}")
}
```

### Async with error handling

```amalgame
public async Task<Result<User>> SafeFetch(string id) {
    try {
        let user = await FetchUser(id)
        return Result.Ok(user)
    } catch (NetworkException e) {
        return Result.Error(e.Message)
    }
}
```

### Parallel execution

```amalgame
let task1 = FetchUser("1")
let task2 = FetchUser("2")
let task3 = FetchUser("3")

let results = await Task.All([task1, task2, task3])
```

---

## 21. Error Handling

### `Result<T>`

```amalgame
public Result<int> Divide(int a, int b) {
    if (b == 0) {
        return Result.Error("Division by zero")
    }
    return Result.Ok(a / b)
}

let result = Divide(10, 2)

match result {
    Result.Ok(value)    => Console.WriteLine("Result: {value}"),
    Result.Error(msg)   => Console.WriteLine("Error: {msg}")
}
```

### `Option<T>`

```amalgame
public Option<Player> FindPlayer(string name) {
    foreach (let p in this.Players) {
        if (p.Name == name) {
            return Option.Some(p)
        }
    }
    return Option.None()
}

let found = FindPlayer("Arthus")

match found {
    Option.Some(p)  => Console.WriteLine("Found: {p.Name}"),
    Option.None()   => Console.WriteLine("Not found")
}
```

### `try` / `catch`

```amalgame
try {
    let content = File.ReadAll("data.json")
    let parsed  = Json.Parse(content)
    Console.WriteLine("Loaded {parsed.Count} items")
} catch (FileNotFoundException e) {
    Console.WriteLine("File not found: {e.Path}")
} catch (JsonException e) {
    Console.WriteLine("Parse error: {e.Message}")
}
```

---

## 22. Collections

### List

```amalgame
let names = new List<string>()
names.Add("Arthus")
names.Add("Merlin")
names.Add("Robyn")

Console.WriteLine(names.Count)    // 3
Console.WriteLine(names[0])       // Arthus

// List literal
let scores = [10, 20, 30, 40, 50]
```

### Map

```amalgame
let levels = new Map<string, int>()
levels["Arthus"] = 42
levels["Merlin"]  = 38

Console.WriteLine(levels["Arthus"])   // 42
```

### Set

```amalgame
let tags = new Set<string>()
tags.Add("warrior")
tags.Add("tank")
tags.Add("warrior")   // duplicate — ignored

Console.WriteLine(tags.Count)   // 2
```

### Collection operations

```amalgame
let players = new List<Player>(…)

// Filter
let veterans = players |> Where(p => p.Level >= 35)

// Transform
let names = players |> Select(p => p.Name)

// Sort
let ranked = players |> OrderBy(p => p.Level)
let top    = players |> OrderByDescending(p => p.Score)

// Aggregate
let total = players |> Sum(p => p.Score)
let avg   = players |> Average(p => p.Level)
let count = players |> Count(p => p.IsAlive())

// Single elements
let first = players |> First()
let last  = players |> Last()
let any   = players |> Any(p => p.Level >= 50)
let all   = players |> All(p => p.IsAlive())
```

### List comprehensions

```amalgame
let squares     = [x * x for x in 1..10]
let evenSquares = [x * x for x in 1..10 if x % 2 == 0]
let names       = [p.Name for p in players if p.Level >= 35]
```

---

## 23. Decorators

Decorators attach metadata or behavior to declarations:

```amalgame
@deprecated("Use NewMethod instead")
public void OldMethod() { … }

@override
public string ToString() { … }

@pure
public int Compute(int x) => x * x

@memory(arc)
let shared = new SharedResource()
```

### Built-in decorators

| Decorator | Target | Description |
|---|---|---|
| `@override` | method | Marks method as overriding parent |
| `@pure` | method | No side effects guarantee |
| `@deprecated(msg)` | any | Marks as deprecated |
| `@memory(mode)` | variable | Memory allocation strategy |
| `@inline` | method | Hint to inline at call site |
| `@test` | method | Marks as a test function |

---

## 24. Memory Management

By default, Amalgame uses the **Boehm garbage collector** — safe and automatic.

For performance-critical code, choose your allocation strategy per variable:

```amalgame
// GC — automatic (default)
let player = new Player("Arthus")

// ARC — reference counted (deterministic cleanup)
@arc let resource = new DatabaseConnection()

// Stack — zero allocation overhead
@stack let point = new Point(3.0, 4.0)

// Arena — batch-freed allocator (great for parsers, game frames)
@arena let buffer = new Buffer(65536)
@arena let nodes  = new List<AstNode>()
// ... use buffer and nodes ...
arena.Free()   // frees all arena-allocated objects at once
```

### When to use what

| Strategy | Use when |
|---|---|
| GC (default) | General purpose — correctness first |
| `@arc` | Shared ownership with deterministic cleanup |
| `@stack` | Small, short-lived value types |
| `@arena` | Batch of objects with the same lifetime |

---

## 25. Modules & Imports

### Standard library modules

```amalgame
import Amalgame.IO          // Console, File, Stream
import Amalgame.Net         // Http, WebSocket, Tcp
import Amalgame.Collections // List, Map, Set, Queue
import Amalgame.Math        // Math functions and constants
import Amalgame.Json        // JSON parsing/serialization
import Amalgame.Time        // DateTime, Duration, Timer
import Amalgame.Async       // Task, Channel, Goroutine
```

### Import with alias

```amalgame
import Amalgame.Collections as Col
import MyApp.Models as M

let list = new Col.List<M.Player>()
```

### Namespace organization

```amalgame
// File: src/models/Player.am
namespace MyApp.Models

public class Player { … }
public record Stats(int Kills, int Deaths)
```

```amalgame
// File: src/services/GameService.am
namespace MyApp.Services

import MyApp.Models

public class GameService {
    public Player FindPlayer(string name) { … }
}
```

---

## 26. Compiler Pipeline

```
source.am
    │
    ▼
[ Lexer ]        → tokens (identifiers, keywords, literals, operators)
    │
    ▼
[ Parser ]       → AST (Abstract Syntax Tree)
    │
    ▼
[ Resolver ]     → AST + SymbolTable
                   (name resolution, scope checking, "did you mean?" hints)
    │
    ▼
[ TypeChecker ]  → annotated AST
                   (type inference, compatibility checks, null safety)
    │
    ▼
[ C Generator ]  → output.c
                   (struct mapping, function naming, runtime calls)
    │
    ▼
[ GCC ]          → native executable
```

### What each pass does

**Lexer** — Converts source text to a flat list of tokens. Tracks line and column for error messages.

**Parser** — Recursive descent parser that builds the AST. Reports syntax errors with exact position.

**Resolver (Pass 1)** — Collects all top-level declarations (classes, records, enums) into the global symbol table, enabling forward references.

**Resolver (Pass 2)** — Walks the full AST, resolves every identifier, validates scopes, checks `break`/`continue`/`await` context, and reports "did you mean X?" suggestions using Levenshtein distance.

**TypeChecker** — Infers types for every expression, validates compatibility at every assignment and call site, checks null safety, and annotates symbols with their resolved types.

**C Generator** — Emits clean, readable C code. Classes become structs, methods become `ClassName_methodName()` functions, inheritance becomes struct embedding.

---

## 27. Error Messages

All errors follow a consistent format:

```
┌── [phase] file.am:line:column
│
│  Clear, human-readable error message
│
└──
```

### Examples

**Parser error:**
```
┌── Erreur dans hello.am:6:29
│
│  Attendu un identifiant, trouvé ')'
│
└──
```

**Resolver error (with suggestion):**
```
┌── [resolver] player.am:12:5
│
│  Unknown symbol 'playr' — did you mean 'player'?
│
└──
```

**TypeChecker error:**
```
┌── [typechecker] game.am:24:16
│
│  Return type mismatch: expected 'int', got 'string'
│
└──
```

---

## 28. Current Limitations

As of v0.3.0, the following features are parsed and type-checked but not yet fully compiled to C:

| Feature | Status |
|---|---|
| Full generics (type unification) | 🔜 Planned v0.4.0 |
| Interface vtable dispatch | 🔜 Planned v0.4.0 |
| Async / coroutines | 🔜 Planned v0.5.0 |
| Goroutines (`go` statement) | 🔜 Planned v0.5.0 |
| Exception propagation | 🔜 Planned v0.4.0 |
| Standard library | 🔜 Planned v0.4.0 |
| LSP server | 🔜 Planned v0.5.0 |
| Multi-file compilation | 🔜 Planned v0.4.0 |

Multi-line function call arguments are not yet supported:

```amalgame
// ❌ Not yet supported
let result = SomeFunction(
    arg1,
    arg2
)

// ✅ Use intermediate variables instead
let r = SomeFunction(arg1, arg2)
```

---

## Quick Reference

### Keywords

```
namespace   import      class       extends     implements
interface   trait       enum        record      data
public      private     protected   internal
static      abstract    override    virtual     async
pure        weak
let         var         if          else        match
while       for         foreach     in          return
guard       break       continue    try         catch
go          select      case        timeout     with
new         this        null        true        false
await       is
```

### Built-in types

```
int   float   double   bool   string   char   byte   void
i8    i16     i32      i64
u8    u16     u32      u64
f32   f64
```

### Naming conventions

| Element | Convention | Example |
|---|---|---|
| Classes, types | `PascalCase` | `PlayerManager` |
| Methods | `PascalCase` | `FindPlayer()` |
| Fields, variables | `camelCase` | `playerName` |
| Constants | `SCREAMING_SNAKE` | `MAX_PLAYERS` |
| Namespaces | `PascalCase.PascalCase` | `MyApp.Models` |
| Files | `PascalCase.am` | `PlayerManager.am` |

---

*Amalgame v0.3.0 — Copyright © 2026 Bastien MOUGET — Apache 2.0*

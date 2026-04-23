// ═══════════════════════════════════════════════════════
//  ast.vala  -  Abstract Syntax Tree du langage CODE
//
//  Organisation :
//  1. Classe de base AstNode
//  2. Nœuds Programme (Program, Namespace, Import)
//  3. Nœuds Déclaration (Class, Interface, Enum...)
//  4. Nœuds Instruction (If, Match, While, For...)
//  5. Nœuds Expression  (Binary, Call, Literal...)
//  6. Nœuds Type        (SimpleType, GenericType...)
// ═══════════════════════════════════════════════════════

namespace CodeTranspiler.Ast {

    using CodeTranspiler.Lexer;


    // ═══════════════════════════════════════════════════
    //  1. CLASSE DE BASE
    // ═══════════════════════════════════════════════════

    /**
     * Nœud de base de l'AST.
     * Tous les nœuds héritent de AstNode.
     *
     * Contient :
     * - Position dans le source (pour erreurs et debug)
     * - Référence au token d'origine
     * - Lien parent (pour remontée dans l'arbre)
     */
    public abstract class AstNode : Object {

        /** Token d'origine dans le source */
        public Token?    SourceToken { get; set; }

        /** Ligne dans le fichier source */
        public int       Line        { get; set; }

        /** Colonne dans le fichier source */
        public int       Column      { get; set; }

        /** Fichier source d'origine */
        public string    Filename    { get; set; }

        /** Nœud parent dans l'arbre */
        public weak AstNode? Parent { get; set; }

        /**
         * Accepte un visiteur (Visitor pattern).
         * Chaque sous-classe implémente cette méthode.
         */
        public abstract void Accept(AstVisitor visitor);

        /**
         * Copie la position depuis un token.
         */
        public void SetPosition(Token token) {
            SourceToken = token;
            Line        = token.Line;
            Column      = token.Column;
            Filename    = token.Filename;
        }

        public string Location() {
            return "%s:%d:%d".printf(Filename, Line, Column);
        }
    }


    // ═══════════════════════════════════════════════════
    //  2. NŒUDS PROGRAMME
    // ═══════════════════════════════════════════════════

    /**
     * Racine de l'AST.
     * Représente un fichier .code complet.
     *
     * Grammaire :
     *   Program = NamespaceDecl { ImportDecl } { TopLevelDecl }
     */
    public class ProgramNode : AstNode {

        public NamespaceNode              Namespace    { get; set; }
        public Gee.ArrayList<ImportNode>  Imports      { get; set; }
        public Gee.ArrayList<AstNode>     Declarations { get; set; }

        public ProgramNode() {
            Imports      = new Gee.ArrayList<ImportNode>();
            Declarations = new Gee.ArrayList<AstNode>();
        }

        public override void Accept(AstVisitor v) {
            v.VisitProgram(this);
        }
    }

    /**
     * Déclaration de namespace.
     *   namespace MyApp.Models
     */
    public class NamespaceNode : AstNode {

        public string Name { get; set; }   // "MyApp.Models"

        public NamespaceNode(string name) {
            Name = name;
        }

        public override void Accept(AstVisitor v) {
            v.VisitNamespace(this);
        }
    }

    /**
     * Déclaration d'import.
     *   import Code.IO
     *   import MyApp.Models as Models
     */
    public class ImportNode : AstNode {

        public string  Name  { get; set; }   // "Code.IO"
        public string? Alias { get; set; }   // "IO" si "import Code.IO as IO"

        public ImportNode(string name, string? alias = null) {
            Name  = name;
            Alias = alias;
        }

        public override void Accept(AstVisitor v) {
            v.VisitImport(this);
        }
    }


    // ═══════════════════════════════════════════════════
    //  3. NŒUDS DÉCLARATION
    // ═══════════════════════════════════════════════════

    /**
     * Décorateur sur une déclaration.
     *   @memory(arc)
     *   @pure
     *   @deprecated("Use NewMethod instead")
     */
    public class DecoratorNode : AstNode {

        public string                    Name { get; set; }
        public Gee.ArrayList<AstNode>    Args { get; set; }

        public DecoratorNode(string name) {
            Name = name;
            Args = new Gee.ArrayList<AstNode>();
        }

        public override void Accept(AstVisitor v) {
            v.VisitDecorator(this);
        }
    }

    /**
     * Modificateur d'accès.
     */
    public enum AccessModifier {
        PUBLIC,
        PRIVATE,
        PROTECTED,
        INTERNAL
    }

    /**
     * Déclaration de classe.
     *
     *   public class Player extends Entity
     *                        implements IDamageable {
     *       ...
     *   }
     */
    public class ClassDeclNode : AstNode {

        public Gee.ArrayList<DecoratorNode>  Decorators  { get; set; }
        public AccessModifier                Access      { get; set; }
        public bool                          IsAbstract  { get; set; }
        public bool                          IsSealed    { get; set; }
        public string                        Name        { get; set; }
        public Gee.ArrayList<GenericParam>   Generics    { get; set; }
        public TypeNode?                     BaseClass   { get; set; }
        public Gee.ArrayList<TypeNode>       Interfaces  { get; set; }
        public Gee.ArrayList<AstNode>        Members     { get; set; }

        public ClassDeclNode(string name) {
            Name       = name;
            Decorators = new Gee.ArrayList<DecoratorNode>();
            Generics   = new Gee.ArrayList<GenericParam>();
            Interfaces = new Gee.ArrayList<TypeNode>();
            Members    = new Gee.ArrayList<AstNode>();
        }

        public override void Accept(AstVisitor v) {
            v.VisitClassDecl(this);
        }
    }

    /**
     * Déclaration d'interface.
     *
     *   public interface IDamageable {
     *       void TakeDamage(int amount)
     *       bool IsAlive()
     *   }
     */
    public class InterfaceDeclNode : AstNode {

        public Gee.ArrayList<DecoratorNode>  Decorators { get; set; }
        public AccessModifier                Access     { get; set; }
        public string                        Name       { get; set; }
        public Gee.ArrayList<GenericParam>   Generics   { get; set; }
        public Gee.ArrayList<TypeNode>       BaseTypes  { get; set; }
        public Gee.ArrayList<AstNode>        Members    { get; set; }

        public InterfaceDeclNode(string name) {
            Name       = name;
            Decorators = new Gee.ArrayList<DecoratorNode>();
            Generics   = new Gee.ArrayList<GenericParam>();
            BaseTypes  = new Gee.ArrayList<TypeNode>();
            Members    = new Gee.ArrayList<AstNode>();
        }

        public override void Accept(AstVisitor v) {
            v.VisitInterfaceDecl(this);
        }
    }

    /**
     * Déclaration d'enum.
     *
     *   public enum Role {
     *       Tank,
     *       Healer,
     *       Mage
     *   }
     */
    public class EnumDeclNode : AstNode {

        public Gee.ArrayList<DecoratorNode>  Decorators { get; set; }
        public AccessModifier                Access     { get; set; }
        public string                        Name       { get; set; }
        public Gee.ArrayList<EnumMemberNode> Members    { get; set; }
        public Gee.ArrayList<MethodDeclNode> Methods    { get; set; }

        public EnumDeclNode(string name) {
            Name       = name;
            Decorators = new Gee.ArrayList<DecoratorNode>();
            Members    = new Gee.ArrayList<EnumMemberNode>();
            Methods    = new Gee.ArrayList<MethodDeclNode>();
        }

        public override void Accept(AstVisitor v) {
            v.VisitEnumDecl(this);
        }
    }

    /**
     * Membre d'un enum.
     *   Tank
     *   Success(int statusCode, string body)
     */
    public class EnumMemberNode : AstNode {

        public string                   Name        { get; set; }
        public Gee.ArrayList<TypeNode>  AssocTypes  { get; set; }

        public EnumMemberNode(string name) {
            Name       = name;
            AssocTypes = new Gee.ArrayList<TypeNode>();
        }

        public override void Accept(AstVisitor v) {
            v.VisitEnumMember(this);
        }
    }

    /**
     * Déclaration de record (immuable).
     *
     *   public record Point(float X, float Y)
     */
    public class RecordDeclNode : AstNode {

        public Gee.ArrayList<DecoratorNode>  Decorators { get; set; }
        public AccessModifier                Access     { get; set; }
        public string                        Name       { get; set; }
        public Gee.ArrayList<RecordParamNode> Params   { get; set; }
        public Gee.ArrayList<MethodDeclNode> Methods   { get; set; }

        public RecordDeclNode(string name) {
            Name       = name;
            Decorators = new Gee.ArrayList<DecoratorNode>();
            Params     = new Gee.ArrayList<RecordParamNode>();
            Methods    = new Gee.ArrayList<MethodDeclNode>();
        }

        public override void Accept(AstVisitor v) {
            v.VisitRecordDecl(this);
        }
    }

    /**
     * Déclaration de data class.
     *
     *   public data class Player(string Name, int Health = 100)
     */
    public class DataClassDeclNode : AstNode {

        public Gee.ArrayList<DecoratorNode>   Decorators { get; set; }
        public AccessModifier                 Access     { get; set; }
        public string                         Name       { get; set; }
        public Gee.ArrayList<RecordParamNode> Params     { get; set; }
        public Gee.ArrayList<AstNode>         Members    { get; set; }

        public DataClassDeclNode(string name) {
            Name       = name;
            Decorators = new Gee.ArrayList<DecoratorNode>();
            Params     = new Gee.ArrayList<RecordParamNode>();
            Members    = new Gee.ArrayList<AstNode>();
        }

        public override void Accept(AstVisitor v) {
            v.VisitDataClassDecl(this);
        }
    }

    /**
     * Paramètre d'un record ou data class.
     *   string Name
     *   int Health = 100
     */
    public class RecordParamNode : AstNode {

        public TypeNode  ParamType   { get; set; }
        public string    Name        { get; set; }
        public AstNode?  Default     { get; set; }

        public RecordParamNode(TypeNode type, string name,
                               AstNode? defaultVal = null) {
            ParamType = type;
            Name      = name;
            Default   = defaultVal;
        }

        public override void Accept(AstVisitor v) {
            v.VisitRecordParam(this);
        }
    }

    /**
     * Déclaration de champ (field).
     *
     *   public string Name
     *   private int _health = 100
     *   weak Player? Parent
     */
    public class FieldDeclNode : AstNode {

        public Gee.ArrayList<DecoratorNode> Decorators { get; set; }
        public AccessModifier               Access     { get; set; }
        public bool                         IsStatic   { get; set; }
        public bool                         IsWeak     { get; set; }
        public string                       Name       { get; set; }
        public TypeNode                     FieldType  { get; set; }
        public AstNode?                     Initial    { get; set; }

        public FieldDeclNode(string name, TypeNode type) {
            Name       = name;
            FieldType  = type;
            Decorators = new Gee.ArrayList<DecoratorNode>();
        }

        public override void Accept(AstVisitor v) {
            v.VisitFieldDecl(this);
        }
    }

    /**
     * Déclaration de propriété.
     *
     *   public string Name { get; set; }
     *   public float Area  { get => Math.PI * R * R }
     */
    public class PropertyDeclNode : AstNode {

        public Gee.ArrayList<DecoratorNode> Decorators   { get; set; }
        public AccessModifier               Access       { get; set; }
        public bool                         IsStatic     { get; set; }
        public string                       Name         { get; set; }
        public TypeNode                     PropType     { get; set; }
        public AstNode?                     Getter       { get; set; }
        public AstNode?                     Setter       { get; set; }
        public AstNode?                     Initial      { get; set; }

        public PropertyDeclNode(string name, TypeNode type) {
            Name       = name;
            PropType   = type;
            Decorators = new Gee.ArrayList<DecoratorNode>();
        }

        public override void Accept(AstVisitor v) {
            v.VisitPropertyDecl(this);
        }
    }

    /**
     * Déclaration de méthode ou fonction.
     *
     *   public async Task<List<User>> GetUsers(int page = 1) {
     *       ...
     *   }
     *
     *   public pure int Add(int a, int b) => a + b
     */
    public class MethodDeclNode : AstNode {

        public Gee.ArrayList<DecoratorNode> Decorators  { get; set; }
        public AccessModifier               Access      { get; set; }
        public bool                         IsStatic    { get; set; }
        public bool                         IsAbstract  { get; set; }
        public bool                         IsOverride  { get; set; }
        public bool                         IsAsync     { get; set; }
        public bool                         IsPure      { get; set; }
        public string                       Name        { get; set; }
        public Gee.ArrayList<GenericParam>  Generics    { get; set; }
        public Gee.ArrayList<ParamNode>     Params      { get; set; }
        public TypeNode?                    ReturnType  { get; set; }
        public AstNode?                     Body        { get; set; }
        // Body est soit un BlockNode soit une ExpressionNode (=>)

        public MethodDeclNode(string name) {
            Name       = name;
            Decorators = new Gee.ArrayList<DecoratorNode>();
            Generics   = new Gee.ArrayList<GenericParam>();
            Params     = new Gee.ArrayList<ParamNode>();
        }

        public override void Accept(AstVisitor v) {
            v.VisitMethodDecl(this);
        }
    }

    /**
     * Déclaration de constructeur.
     *
     *   public Player(string name, int health = 100) {
     *       this.Name   = name
     *       this.Health = health
     *   }
     */
    public class ConstructorDeclNode : AstNode {

        public Gee.ArrayList<DecoratorNode> Decorators { get; set; }
        public AccessModifier               Access     { get; set; }
        public Gee.ArrayList<ParamNode>     Params     { get; set; }
        public BlockNode                    Body       { get; set; }

        public ConstructorDeclNode() {
            Decorators = new Gee.ArrayList<DecoratorNode>();
            Params     = new Gee.ArrayList<ParamNode>();
        }

        public override void Accept(AstVisitor v) {
            v.VisitConstructorDecl(this);
        }
    }

    /**
     * Paramètre d'une méthode ou fonction.
     *   int amount
     *   string name = "default"
     *   weak Player? parent
     */
    public class ParamNode : AstNode {

        public bool     IsWeak     { get; set; }
        public string   Name       { get; set; }
        public TypeNode ParamType  { get; set; }
        public AstNode? Default    { get; set; }

        public ParamNode(string name, TypeNode type,
                         AstNode? defaultVal = null) {
            Name      = name;
            ParamType = type;
            Default   = defaultVal;
        }

        public override void Accept(AstVisitor v) {
            v.VisitParam(this);
        }
    }

    /**
     * Paramètre générique.
     *   T
     *   T : IComparable<T>
     */
    public class GenericParam : Object {
        public string                  Name        { get; set; }
        public Gee.ArrayList<TypeNode> Constraints { get; set; }

        public GenericParam(string name) {
            Name        = name;
            Constraints = new Gee.ArrayList<TypeNode>();
        }
    }


    // ═══════════════════════════════════════════════════
    //  4. NŒUDS INSTRUCTION (Statements)
    // ═══════════════════════════════════════════════════

    /**
     * Bloc d'instructions.
     *   { stmt1; stmt2; ... }
     */
    public class BlockNode : AstNode {

        public Gee.ArrayList<AstNode> Statements { get; set; }

        public BlockNode() {
            Statements = new Gee.ArrayList<AstNode>();
        }

        public override void Accept(AstVisitor v) {
            v.VisitBlock(this);
        }
    }

    /**
     * Déclaration de variable locale.
     *   let name = "World"
     *   var counter: int = 0
     *   let@arena buf = new Buffer()
     */
    public class VarDeclNode : AstNode {

        public bool     IsLet      { get; set; }  // let = immuable
        public string?  MemoryHint { get; set; }  // @arena, @arc, @stack...
        public string   Name       { get; set; }
        public TypeNode? VarType   { get; set; }  // null = inféré
        public AstNode?  Initial   { get; set; }

        public VarDeclNode(bool isLet, string name) {
            IsLet = isLet;
            Name  = name;
        }

        public override void Accept(AstVisitor v) {
            v.VisitVarDecl(this);
        }
    }

    /**
     * Instruction if/else.
     *
     *   if (player.IsAlive()) {
     *       ...
     *   } else if (player.Health < 10) {
     *       ...
     *   } else {
     *       ...
     *   }
     */
    public class IfNode : AstNode {

        public AstNode               Condition  { get; set; }
        public BlockNode             ThenBlock  { get; set; }
        public Gee.ArrayList<ElseIfNode> ElseIfs { get; set; }
        public BlockNode?            ElseBlock  { get; set; }

        public IfNode(AstNode condition, BlockNode then) {
            Condition = condition;
            ThenBlock = then;
            ElseIfs   = new Gee.ArrayList<ElseIfNode>();
        }

        public override void Accept(AstVisitor v) {
            v.VisitIf(this);
        }
    }

    /** Branche else if */
    public class ElseIfNode : AstNode {
        public AstNode   Condition { get; set; }
        public BlockNode Block     { get; set; }

        public ElseIfNode(AstNode condition, BlockNode block) {
            Condition = condition;
            Block     = block;
        }

        public override void Accept(AstVisitor v) {
            v.VisitElseIf(this);
        }
    }

    /**
     * Instruction match.
     *
     *   match player.Health {
     *       100        => "Full health",
     *       75..99     => "Slightly wounded",
     *       0          => "Dead",
     *       _          => "Unknown"
     *   }
     */
    public class MatchNode : AstNode {

        public AstNode                    Subject { get; set; }
        public Gee.ArrayList<MatchArmNode> Arms   { get; set; }

        public MatchNode(AstNode subject) {
            Subject = subject;
            Arms    = new Gee.ArrayList<MatchArmNode>();
        }

        public override void Accept(AstVisitor v) {
            v.VisitMatch(this);
        }
    }

    /**
     * Bras d'un match.
     *   75..99 => "Slightly wounded",
     */
    public class MatchArmNode : AstNode {

        public MatchPatternNode Pattern { get; set; }
        public AstNode          Body    { get; set; }
        // Body = ExpressionNode ou BlockNode

        public MatchArmNode(MatchPatternNode pattern, AstNode body) {
            Pattern = pattern;
            Body    = body;
        }

        public override void Accept(AstVisitor v) {
            v.VisitMatchArm(this);
        }
    }

    /**
     * Pattern dans un match.
     */
    public enum MatchPatternKind {
        WILDCARD,       // _
        LITERAL,        // 42, "hello", true
        RANGE,          // 1..10
        TYPE,           // Player p
        DESTRUCTURE,    // Player(name, health)
        GUARD,          // n if n > 0
        ENUM_VARIANT    // Role.Tank
    }

    public class MatchPatternNode : AstNode {

        public MatchPatternKind               Kind        { get; set; }
        public AstNode?                       Value       { get; set; }
        public AstNode?                       RangeEnd    { get; set; }
        public TypeNode?                      PatternType { get; set; }
        public string?                        BindName    { get; set; }
        public Gee.ArrayList<MatchPatternNode> SubPatterns { get; set; }
        public AstNode?                        Guard       { get; set; }

        public MatchPatternNode(MatchPatternKind kind) {
            Kind        = kind;
            SubPatterns = new Gee.ArrayList<MatchPatternNode>();
        }

        public override void Accept(AstVisitor v) {
            v.VisitMatchPattern(this);
        }
    }

    /**
     * Boucle while.
     *   while (counter < 10) { ... }
     */
    public class WhileNode : AstNode {

        public AstNode   Condition { get; set; }
        public BlockNode Body      { get; set; }

        public WhileNode(AstNode condition, BlockNode body) {
            Condition = condition;
            Body      = body;
        }

        public override void Accept(AstVisitor v) {
            v.VisitWhile(this);
        }
    }

    /**
     * Boucle for classique.
     *   for (let i = 0; i < 10; i++) { ... }
     */
    public class ForNode : AstNode {

        public VarDeclNode Init      { get; set; }
        public AstNode     Condition { get; set; }
        public AstNode     Step      { get; set; }
        public BlockNode   Body      { get; set; }

        public ForNode(VarDeclNode init, AstNode condition,
                       AstNode step, BlockNode body) {
            Init      = init;
            Condition = condition;
            Step      = step;
            Body      = body;
        }

        public override void Accept(AstVisitor v) {
            v.VisitFor(this);
        }
    }

    /**
     * Boucle foreach.
     *   foreach (let player in players) { ... }
     */
    public class ForeachNode : AstNode {

        public bool      IsLet      { get; set; }
        public string    VarName    { get; set; }
        public AstNode   Collection { get; set; }
        public BlockNode Body       { get; set; }

        public ForeachNode(bool isLet, string varName,
                           AstNode collection, BlockNode body) {
            IsLet      = isLet;
            VarName    = varName;
            Collection = collection;
            Body       = body;
        }

        public override void Accept(AstVisitor v) {
            v.VisitForeach(this);
        }
    }

    /**
     * Instruction return.
     *   return player
     *   return          (void)
     */
    public class ReturnNode : AstNode {

        public AstNode? Value { get; set; }

        public ReturnNode(AstNode? value = null) {
            Value = value;
        }

        public override void Accept(AstVisitor v) {
            v.VisitReturn(this);
        }
    }

    /**
     * Guard clause.
     *   guard player != null else { return }
     */
    public class GuardNode : AstNode {

        public AstNode   Condition { get; set; }
        public BlockNode ElseBlock { get; set; }

        public GuardNode(AstNode condition, BlockNode elseBlock) {
            Condition = condition;
            ElseBlock = elseBlock;
        }

        public override void Accept(AstVisitor v) {
            v.VisitGuard(this);
        }
    }

    /**
     * Break / Continue.
     */
    public class BreakNode    : AstNode {
        public override void Accept(AstVisitor v) { v.VisitBreak(this); }
    }

    public class ContinueNode : AstNode {
        public override void Accept(AstVisitor v) { v.VisitContinue(this); }
    }

    /**
     * Try / Catch.
     *   try { ... } catch (Error e) { ... }
     */
    public class TryCatchNode : AstNode {

        public BlockNode TryBlock   { get; set; }
        public string    ErrorType  { get; set; }
        public string    ErrorName  { get; set; }
        public BlockNode CatchBlock { get; set; }

        public TryCatchNode(BlockNode tryBlock,
                            string    errorType,
                            string    errorName,
                            BlockNode catchBlock) {
            TryBlock   = tryBlock;
            ErrorType  = errorType;
            ErrorName  = errorName;
            CatchBlock = catchBlock;
        }

        public override void Accept(AstVisitor v) {
            v.VisitTryCatch(this);
        }
    }

    /**
     * Lancement d'une goroutine.
     *   go ProcessImages(list)
     */
    public class GoStmtNode : AstNode {

        public AstNode Expression { get; set; }

        public GoStmtNode(AstNode expression) {
            Expression = expression;
        }

        public override void Accept(AstVisitor v) {
            v.VisitGoStmt(this);
        }
    }


    // ═══════════════════════════════════════════════════
    //  5. NŒUDS EXPRESSION
    // ═══════════════════════════════════════════════════

    /**
     * Expression binaire.
     *   a + b
     *   x == y
     *   list |> Where(...)
     */
    public class BinaryExprNode : AstNode {

        public AstNode Left     { get; set; }
        public string  Operator { get; set; }  // "+", "==", "|>", "??"...
        public AstNode Right    { get; set; }

        public BinaryExprNode(AstNode left, string op, AstNode right) {
            Left     = left;
            Operator = op;
            Right    = right;
        }

        public override void Accept(AstVisitor v) {
            v.VisitBinaryExpr(this);
        }
    }

    /**
     * Expression unaire.
     *   !valid
     *   -count
     */
    public class UnaryExprNode : AstNode {

        public string  Operator { get; set; }
        public AstNode Operand  { get; set; }
        public bool    IsPrefix { get; set; }

        public UnaryExprNode(string op, AstNode operand,
                             bool isPrefix = true) {
            Operator = op;
            Operand  = operand;
            IsPrefix = isPrefix;
        }

        public override void Accept(AstVisitor v) {
            v.VisitUnaryExpr(this);
        }
    }

    /**
     * Accès à un membre.
     *   player.Name
     *   player?.Health    (null-safe)
     */
    public class MemberAccessNode : AstNode {

        public AstNode Target     { get; set; }
        public string  MemberName { get; set; }
        public bool    IsNullSafe { get; set; }   // ?.

        public MemberAccessNode(AstNode target, string member,
                                bool nullSafe = false) {
            Target     = target;
            MemberName = member;
            IsNullSafe = nullSafe;
        }

        public override void Accept(AstVisitor v) {
            v.VisitMemberAccess(this);
        }
    }

    /**
     * Appel de méthode ou fonction.
     *   Console.WriteLine("hello")
     *   player.TakeDamage(10)
     *   Add(1, 2)
     */
    public class CallExprNode : AstNode {

        public AstNode               Callee     { get; set; }
        public Gee.ArrayList<AstNode> Arguments { get; set; }
        public Gee.ArrayList<TypeNode> Generics { get; set; }
        // Arguments nommés : "name: value"
        public Gee.HashMap<string, AstNode> NamedArgs { get; set; }

        public CallExprNode(AstNode callee) {
            Callee    = callee;
            Arguments = new Gee.ArrayList<AstNode>();
            Generics  = new Gee.ArrayList<TypeNode>();
            NamedArgs = new Gee.HashMap<string, AstNode>();
        }

        public override void Accept(AstVisitor v) {
            v.VisitCallExpr(this);
        }
    }

    /**
     * Accès à un index.
     *   list[0]
     *   map["key"]
     */
    public class IndexExprNode : AstNode {

        public AstNode Target { get; set; }
        public AstNode Index  { get; set; }

        public IndexExprNode(AstNode target, AstNode index) {
            Target = target;
            Index  = index;
        }

        public override void Accept(AstVisitor v) {
            v.VisitIndexExpr(this);
        }
    }

    /**
     * Expression d'affectation.
     *   x = 5
     *   player.Health += 10
     */
    public class AssignExprNode : AstNode {

        public AstNode Target   { get; set; }
        public string  Operator { get; set; }  // "=", "+=", "-="...
        public AstNode Value    { get; set; }

        public AssignExprNode(AstNode target, string op, AstNode value) {
            Target   = target;
            Operator = op;
            Value    = value;
        }

        public override void Accept(AstVisitor v) {
            v.VisitAssignExpr(this);
        }
    }

    /**
     * Création d'objet.
     *   new Player("Arthus")
     *   new List<int>()
     */
    public class NewExprNode : AstNode {

        public TypeNode              ObjectType { get; set; }
        public Gee.ArrayList<AstNode> Arguments { get; set; }
        public Gee.HashMap<string, AstNode> NamedArgs { get; set; }

        public NewExprNode(TypeNode type) {
            ObjectType = type;
            Arguments  = new Gee.ArrayList<AstNode>();
            NamedArgs  = new Gee.HashMap<string, AstNode>();
        }

        public override void Accept(AstVisitor v) {
            v.VisitNewExpr(this);
        }
    }

    /**
     * Expression lambda.
     *   p => p.IsAlive()
     *   (p, q) => p.Level > q.Level
     *   (int x) => x * 2
     */
    public class LambdaExprNode : AstNode {

        public Gee.ArrayList<ParamNode> Params { get; set; }
        public AstNode                  Body   { get; set; }
        // Body = ExpressionNode ou BlockNode

        public LambdaExprNode() {
            Params = new Gee.ArrayList<ParamNode>();
        }

        public override void Accept(AstVisitor v) {
            v.VisitLambdaExpr(this);
        }
    }

    /**
     * Expression await.
     *   await Http.GetAsync(url)
     */
    public class AwaitExprNode : AstNode {

        public AstNode Expression { get; set; }

        public AwaitExprNode(AstNode expression) {
            Expression = expression;
        }

        public override void Accept(AstVisitor v) {
            v.VisitAwaitExpr(this);
        }
    }

    /**
     * Expression "with" (copie avec modification).
     *   player with { Health = 50 }
     */
    public class WithExprNode : AstNode {

        public AstNode                          Source  { get; set; }
        public Gee.HashMap<string, AstNode>     Changes { get; set; }

        public WithExprNode(AstNode source) {
            Source  = source;
            Changes = new Gee.HashMap<string, AstNode>();
        }

        public override void Accept(AstVisitor v) {
            v.VisitWithExpr(this);
        }
    }

    /**
     * Littéral liste.
     *   [1, 2, 3, 4, 5]
     *   [x * x for x in numbers if x > 0]
     */
    public class ListLiteralNode : AstNode {

        public Gee.ArrayList<AstNode> Elements    { get; set; }
        // Compréhension de liste
        public bool     IsComprehension { get; set; }
        public string?  CompVarName     { get; set; }
        public AstNode? CompSource      { get; set; }
        public AstNode? CompFilter      { get; set; }
        public AstNode? CompExpr        { get; set; }

        public ListLiteralNode() {
            Elements = new Gee.ArrayList<AstNode>();
        }

        public override void Accept(AstVisitor v) {
            v.VisitListLiteral(this);
        }
    }

    /**
     * Littéral map (dictionnaire).
     *   { "key": value, ... }
     */
    public class MapLiteralNode : AstNode {

        public Gee.ArrayList<MapEntryNode> Entries { get; set; }

        public MapLiteralNode() {
            Entries = new Gee.ArrayList<MapEntryNode>();
        }

        public override void Accept(AstVisitor v) {
            v.VisitMapLiteral(this);
        }
    }

    public class MapEntryNode : AstNode {
        public AstNode Key   { get; set; }
        public AstNode Value { get; set; }

        public MapEntryNode(AstNode key, AstNode value) {
            Key   = key;
            Value = value;
        }

        public override void Accept(AstVisitor v) {
            v.VisitMapEntry(this);
        }
    }

    /**
     * Référence à un identifiant.
     *   player
     *   Console
     *   myVar
     */
    public class IdentifierNode : AstNode {

        public string Name { get; set; }

        public IdentifierNode(string name) {
            Name = name;
        }

        public override void Accept(AstVisitor v) {
            v.VisitIdentifier(this);
        }
    }

    /**
     * Référence à "this".
     */
    public class ThisNode : AstNode {
        public override void Accept(AstVisitor v) { v.VisitThis(this); }
    }

    /**
     * Littéral null.
     */
    public class NullNode : AstNode {
        public override void Accept(AstVisitor v) { v.VisitNull(this); }
    }

    /**
     * Littéraux simples.
     *   42, 3.14, "hello", true, false
     */
    public enum LiteralKind {
        INTEGER,
        FLOAT,
        STRING,
        BOOL,
        INTERPOLATED_STRING
    }

    public class LiteralNode : AstNode {

        public LiteralKind Kind  { get; set; }
        public string      Raw   { get; set; }  // valeur brute

        // Valeurs typées
        public int64  IntValue    { get; set; }
        public double FloatValue  { get; set; }
        public string StringValue { get; set; }
        public bool   BoolValue   { get; set; }

        // Pour les strings interpolées : "{name} is {age}"
        // → liste de segments texte + expressions
        public Gee.ArrayList<AstNode>? Segments { get; set; }

        public LiteralNode(LiteralKind kind, string raw) {
            Kind = kind;
            Raw  = raw;
        }

        public override void Accept(AstVisitor v) {
            v.VisitLiteral(this);
        }
    }


    // ═══════════════════════════════════════════════════
    //  6. NŒUDS TYPE
    // ═══════════════════════════════════════════════════

    /**
     * Référence à un type.
     * Classe de base pour tous les types.
     */
    public abstract class TypeNode : AstNode { }

    /**
     * Type simple.
     *   int, string, Player, MyApp.Models.User
     */
    public class SimpleTypeNode : TypeNode {

        public string Name       { get; set; }  // "int", "Player"...
        public bool   IsNullable { get; set; }  // string? → true

        public SimpleTypeNode(string name, bool nullable = false) {
            Name       = name;
            IsNullable = nullable;
        }

        public override void Accept(AstVisitor v) {
            v.VisitSimpleType(this);
        }
    }

    /**
     * Type générique.
     *   List<Player>
     *   Map<string, int>
     *   Result<User, Error>
     */
    public class GenericTypeNode : TypeNode {

        public string                  Name       { get; set; }
        public Gee.ArrayList<TypeNode> TypeArgs   { get; set; }
        public bool                    IsNullable { get; set; }

        public GenericTypeNode(string name) {
            Name     = name;
            TypeArgs = new Gee.ArrayList<TypeNode>();
        }

        public override void Accept(AstVisitor v) {
            v.VisitGenericType(this);
        }
    }

    /**
     * Type fonction.
     *   Func<int, int, bool>
     *   Func<string, void>
     */
    public class FuncTypeNode : TypeNode {

        public Gee.ArrayList<TypeNode> ParamTypes  { get; set; }
        public TypeNode                ReturnType  { get; set; }

        public FuncTypeNode(TypeNode returnType) {
            ReturnType = returnType;
            ParamTypes = new Gee.ArrayList<TypeNode>();
        }

        public override void Accept(AstVisitor v) {
            v.VisitFuncType(this);
        }
    }

    /**
     * Type tuple.
     *   (int, string)
     *   (Player, int, bool)
     */
    public class TupleTypeNode : TypeNode {

        public Gee.ArrayList<TypeNode> ElementTypes { get; set; }

        public TupleTypeNode() {
            ElementTypes = new Gee.ArrayList<TypeNode>();
        }

        public override void Accept(AstVisitor v) {
            v.VisitTupleType(this);
        }
    }
}

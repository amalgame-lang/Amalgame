# Générateur C

> **Version** : 0.1.0
> **Fichier**  : `src/transpiler/generator/c_generator.vala`
> **Statut**   : ✅ Fonctionnel (Hello World OK)

---

## Rôle

Transforme l AST en code C valide.

AST  →  [ CGenerator ]  →  fichier.c  →  GCC  →  exécutable


---

## Technique : Visitor Pattern

```vala
public class CGenerator : BaseAstVisitor {

    public override void VisitClassDecl(ClassDeclNode n) {
        // Génère : struct _Player { ... };
    }

    public override void VisitMethodDecl(MethodDeclNode n) {
        // Génère : void Player_method(Player* self) { }
    }
}
# ═══════════════════════════════════════════════════════════
#  Homebrew Formula — Amalgame Language
#  https://github.com/BastienMOUGET/Amalgame
#
#  To publish:
#    1. Fork homebrew-core (or create a tap)
#    2. Place this file at Formula/amalgame.rb
#    3. Submit a Pull Request to homebrew-core
#
#  Or use as a private tap:
#    brew tap BastienMOUGET/amalgame https://github.com/BastienMOUGET/homebrew-amalgame
#    brew install amalgame
# ═══════════════════════════════════════════════════════════

class Amalgame < Formula
  desc "Modern programming language that transpiles to C"
  homepage "https://github.com/BastienMOUGET/Amalgame"
  url "https://github.com/BastienMOUGET/Amalgame/archive/refs/tags/v0.3.0.tar.gz"
  sha256 "REPLACE_WITH_ACTUAL_SHA256_OF_RELEASE_TARBALL"
  license "Apache-2.0"
  head "https://github.com/BastienMOUGET/Amalgame.git", branch: "main"

  # ── Dependencies ──────────────────────────────────────────
  depends_on "meson"  => :build
  depends_on "ninja"  => :build
  depends_on "vala"   => :build
  depends_on "pkg-config" => :build

  depends_on "glib"
  depends_on "libgee"
  depends_on "bdw-gc"
  depends_on "curl"    # for Amalgame.Net HTTP support

  # GCC is provided by macOS Xcode tools or can be installed separately
  # depends_on "gcc" — optional, user likely already has it

  # ── Build ─────────────────────────────────────────────────
  def install
    # Configure with meson
    system "meson", "setup", "build",
           "--prefix=#{prefix}",
           "--buildtype=release"

    # Build
    system "ninja", "-C", "build"

    # Install binary
    bin.install "build/amc"

    # Install runtime header (needed by compiled programs)
    (lib/"amalgame").mkpath
    (lib/"amalgame").install "src/transpiler/runtime/_runtime.h"

    # Install documentation
    doc.install "docs/DEVELOPER_GUIDE.md"
    doc.install "README.md"
  end

  # ── Post-install setup ────────────────────────────────────
  def post_install
    # Create a wrapper script that sets AMC_RUNTIME automatically
    (bin/"amc").unlink if (bin/"amc").exist?

    (bin/"amc").write <<~SHELL
      #!/bin/bash
      export AMC_RUNTIME="#{lib}/amalgame"
      exec "#{lib}/amalgame/amc-bin" "$@"
    SHELL

    (bin/"amc").chmod 0755
  end

  # ── Tests ─────────────────────────────────────────────────
  test do
    # Version check
    assert_match "Amalgame Transpiler v", shell_output("#{bin}/amc --version")

    # Hello World compilation
    (testpath/"hello.am").write <<~AM
      namespace Test
      public class Program {
          public static void Main(string[] args) {
              Console.WriteLine("Hello from Homebrew!")
          }
      }
    AM

    system "#{bin}/amc", "hello.am"
    assert_predicate testpath/"hello", :executable?
    assert_match "Hello from Homebrew!", shell_output("#{testpath}/hello")
  end
end

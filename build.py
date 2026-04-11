#!/usr/bin/env python3
"""
Script para gerenciar build e clean do projeto CMake.
Uso: python build.py [comando] [opções]
"""

import os
import sys
import subprocess
import argparse
from pathlib import Path


class BuildManager:
    def __init__(self, project_root=None):
        self.project_root = Path(project_root or os.getcwd()).resolve()
        self.build_dir = self.project_root / "build"
        self.build_type = "Release"

    def run_command(self, cmd, cwd=None):
        """Executa um comando no terminal."""
        cwd = cwd or self.project_root
        print(f"\n📁 Diretório: {cwd}")
        print(f"🔄 Executando: {' '.join(cmd)}\n")
        
        try:
            result = subprocess.run(
                cmd,
                cwd=cwd,
                check=True
            )
            return result.returncode == 0
        except subprocess.CalledProcessError as e:
            print(f"❌ Erro ao executar comando: {e}")
            return False

    def configure(self, trabalho=None):
        """Configura o projeto com CMake."""
        if not self.build_dir.exists():
            self.build_dir.mkdir(parents=True)
            print(f"✅ Diretório de build criado: {self.build_dir}")

        vcpkg_path = self.project_root / "vcpkg_installed" / "x64-linux" / "share"
        
        cmake_args = [
            "cmake",
            "-B", str(self.build_dir),
            "-S", str(self.project_root),
            f"-DCMAKE_BUILD_TYPE={self.build_type}",
            f"-DCMAKE_PREFIX_PATH={str(vcpkg_path)}",
        ]

        # Configurar quale "trabalho" compilar
        if trabalho:
            for i in range(1, 4):
                cmake_args.append(f"-DBUILD_TRABALHO_0{i}={'ON' if i == trabalho else 'OFF'}")
            print(f"✓ Configurando para compilar: Trabalho {trabalho:02d}")

        return self.run_command(cmake_args)

    def build(self):
        """Compila o projeto."""
        if not self.build_dir.exists():
            print("⚠️  Diretório de build não existe. Execute 'configure' primeiro.")
            return False

        cmd = ["cmake", "--build", str(self.build_dir), "--config", self.build_type, "-j"]
        return self.run_command(cmd)

    def clean(self):
        """Limpa os arquivos compilados."""
        if not self.build_dir.exists():
            print("✅ Diretório de build já não existe.")
            return True

        cmd = ["cmake", "--build", str(self.build_dir), "--target", "clean"]
        success = self.run_command(cmd)
        
        if success:
            print(f"✅ Clean concluído")
        return success

    def clean_all(self):
        """Remove completamente o diretório de build."""
        if self.build_dir.exists():
            import shutil
            print(f"🗑️  Removendo diretório de build: {self.build_dir}")
            shutil.rmtree(self.build_dir)
            print("✅ Diretório de build removido")
            return True
        else:
            print("✅ Diretório de build já não existe.")
            return True

    def rebuild(self, trabalho=None):
        """Remove build anterior e compila novamente."""
        print("🔄 Executando rebuild...")
        self.clean_all()
        self.configure(trabalho)
        return self.build()


def main():
    parser = argparse.ArgumentParser(
        description="Gerenciador de build para projeto CMake",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Exemplos:
  python build.py configure              # Configura o projeto (padrão: trabalho 02)
  python build.py configure --trabalho 1 # Configura para compilar trabalho 01
  python build.py build                  # Compila o projeto
  python build.py clean                  # Limpa objetos compilados
  python build.py clean-all              # Remove diretório de build completamente
  python build.py rebuild                # Remove build anterior e recompila
        """
    )

    parser.add_argument(
        "command",
        choices=["configure", "build", "clean", "clean-all", "rebuild"],
        help="Comando a executar"
    )
    
    parser.add_argument(
        "--trabalho",
        type=int,
        choices=[1, 2, 3],
        default=None,
        help="Qual trabalho compilar (1, 2 ou 3). Padrão: 2"
    )
    
    parser.add_argument(
        "--build-type",
        choices=["Debug", "Release"],
        default="Release",
        help="Tipo de build (Debug ou Release). Padrão: Release"
    )

    args = parser.parse_args()

    manager = BuildManager()
    manager.build_type = args.build_type

    success = False
    if args.command == "configure":
        success = manager.configure(args.trabalho)
    elif args.command == "build":
        success = manager.build()
    elif args.command == "clean":
        success = manager.clean()
    elif args.command == "clean-all":
        success = manager.clean_all()
    elif args.command == "rebuild":
        success = manager.rebuild(args.trabalho)

    if success:
        print("\n✅ Comando concluído com sucesso!")
        sys.exit(0)
    else:
        print("\n❌ Comando falhou!")
        sys.exit(1)


if __name__ == "__main__":
    main()

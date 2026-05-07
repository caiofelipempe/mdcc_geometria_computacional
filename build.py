#!/usr/bin/env python3
"""
Script para gerenciar build e clean do projeto CMake.
Adaptado para suportar Tarefa 02 e Trabalho 01.
"""

import os
import sys
import subprocess
import argparse
import shutil
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

    def configure(self, trabalho=None, tarefa=None):
        """Configura o projeto com CMake mapeando para as variáveis do CMakeLists."""
        if not self.build_dir.exists():
            self.build_dir.mkdir(parents=True)
            print(f"✅ Diretório de build criado: {self.build_dir}")

        # Caminho para dependências do vcpkg em modo manifesto
        vcpkg_path = self.project_root / "vcpkg_installed" / "x64-linux"
        if not vcpkg_path.exists():
            # Tenta um caminho genérico se o x64-linux não existir (ex: Windows ou outras distros)
            vcpkg_path = self.project_root / "vcpkg_installed"

        cmake_args = [
            "cmake",
            "-B", str(self.build_dir),
            "-S", str(self.project_root),
            f"-DCMAKE_BUILD_TYPE={self.build_type}",
            f"-DCMAKE_PREFIX_PATH={str(vcpkg_path)}",
        ]

        # Lógica de ativação baseada no seu CMakeLists.txt
        if trabalho == 1:
            cmake_args.append("-DBUILD_TRABALHO_01=ON")
            print("✓ Ativando: Trabalho 01")
        elif trabalho is not None:
             cmake_args.append("-DBUILD_TRABALHO_01=OFF")

        if tarefa == 2:
            cmake_args.append("-DBUILD_TAREFA_02=ON")
            print("✓ Ativando: Tarefa 02")
        elif tarefa is not None:
            cmake_args.append("-DBUILD_TAREFA_02=OFF")

        return self.run_command(cmake_args)

    def build(self):
        """Compila o projeto."""
        if not self.build_dir.exists():
            print("⚠️  Diretório de build não existe. Execute 'configure' primeiro.")
            return False

        cmd = ["cmake", "--build", str(self.build_dir), "--config", self.build_type, "-j"]
        return self.run_command(cmd)

    def clean_all(self):
        """Remove completamente o diretório de build."""
        if self.build_dir.exists():
            print(f"🗑️  Removendo diretório de build: {self.build_dir}")
            shutil.rmtree(self.build_dir)
            print("✅ Diretório de build removido")
            return True
        print("✅ Diretório de build já não existe.")
        return True

    def rebuild(self, trabalho=None, tarefa=None):
        """Remove build anterior e compila novamente."""
        print("🔄 Executando rebuild...")
        self.clean_all()
        if self.configure(trabalho, tarefa):
            return self.build()
        return False


def main():
    parser = argparse.ArgumentParser(
        description="Gerenciador de build para projeto CMake",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Exemplos:
  python build.py configure --trabalho 1  # Configura Trabalho 01
  python build.py configure --tarefa 2    # Configura Tarefa 02
  python build.py rebuild --trabalho 1    # Limpa e constrói Trabalho 01
  python build.py build                   # Apenas compila o que já está configurado
  python build.py clean-all               # Deleta a pasta build
        """
    )

    parser.add_argument(
        "command",
        choices=["configure", "build", "clean-all", "rebuild"],
        help="Comando a executar"
    )
    
    parser.add_argument(
        "--trabalho",
        type=int,
        choices=[1],
        default=None,
        help="Compilar Trabalho (atualmente suporta: 1)"
    )

    parser.add_argument(
        "--tarefa",
        type=int,
        choices=[2],
        default=None,
        help="Compilar Tarefa (atualmente suporta: 2)"
    )
    
    parser.add_argument(
        "--build-type",
        choices=["Debug", "Release"],
        default="Release",
        help="Tipo de build. Padrão: Release"
    )

    args = parser.parse_args()

    manager = BuildManager()
    manager.build_type = args.build_type

    success = False
    if args.command == "configure":
        success = manager.configure(trabalho=args.trabalho, tarefa=args.tarefa)
    elif args.command == "build":
        success = manager.build()
    elif args.command == "clean-all":
        success = manager.clean_all()
    elif args.command == "rebuild":
        success = manager.rebuild(trabalho=args.trabalho, tarefa=args.tarefa)

    if success:
        print("\n✅ Comando concluído com sucesso!")
        sys.exit(0)
    else:
        print("\n❌ Comando falhou!")
        sys.exit(1)


if __name__ == "__main__":
    main()
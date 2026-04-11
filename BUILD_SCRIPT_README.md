# Build Manager Script

Script Python para gerenciar build e clean do projeto CMake.

## Instalação

O script já está pronto para uso. Requer:
- Python 3.7+
- CMake
- Dependências do projeto (vcpkg)

## Uso

### Comandos disponíveis

```bash
# Configurar o projeto (padrão: trabalho 02, Release)
python build.py configure

# Configurar para compilar um trabalho específico
python build.py configure --trabalho 1    # Trabalho 01
python build.py configure --trabalho 2    # Trabalho 02
python build.py configure --trabalho 3    # Trabalho 03

# Compilar o projeto
python build.py build

# Limpar objetos compilados (mantém CMakeFiles)
python build.py clean

# Remover completamente o diretório de build
python build.py clean-all

# Recompilar do zero (clean-all + configure + build)
python build.py rebuild
python build.py rebuild --trabalho 1      # Rebuild de um trabalho específico

# Debug build (padrão é Release)
python build.py configure --build-type Debug
```

## Exemplos práticos

```bash
# Setup completo do projeto
python build.py configure
python build.py build

# Build e clean rápido
python build.py clean
python build.py build

# Trocar de trabalho
python build.py rebuild --trabalho 1

# Limpeza completa
python build.py clean-all
```

## Opções disponíveis

- `--trabalho {1,2,3}`: Define qual trabalho compilar
- `--build-type {Debug,Release}`: Tipo de compilação (padrão: Release)

## Estrutura de diretórios

Após executar os comandos, será criada:
```
projeto/
├── build/              # Diretório de build (criado automaticamente)
├── CMakeLists.txt
├── build.py           # Este script
└── ...
```

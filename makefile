# Editável
NAME	:= Arcade_Mania
CODE	:= ACMN
MAKER	:= DS



# Local de instalação do devkitPro
DEVKITPRO := /opt/devkitpro
DEVKITARM := $(DEVKITPRO)/devkitARM



# Binários
CC 		:= $(DEVKITARM)/bin/arm-none-eabi-gcc
OBJCOPY := $(DEVKITARM)/bin/arm-none-eabi-objcopy
GBAFIX 	:= $(DEVKITPRO)/tools/bin/gbafix
GRIT	:= $(DEVKITPRO)/tools/bin/grit



# Pastas
BINDIR := bin
INCDIR := include
DATDIR := data
SRCDIR := source
BLDDIR := build



# Pastas que precisam ser criadas
BUILD_DIRS	:= $(BINDIR) $(BLDDIR) $(BLDDIR)/$(DATDIR)










# Arquivos fonte
SOURCES := $(wildcard $(SRCDIR)/*.c)
OBJECTS := $(patsubst $(SRCDIR)/%.c, $(BLDDIR)/%.o, $(SOURCES))



# Arquivos de dados
DATA_BG_FILES		:= $(wildcard $(DATDIR)/BG_*.bmp)
DATA_BG_SOURCES		:= $(patsubst $(DATDIR)/%.bmp, $(BLDDIR)/$(DATDIR)/%.c, $(DATA_BG_FILES))
DATA_BG_OBJECTS		:= $(patsubst $(DATDIR)/%.bmp, $(BLDDIR)/$(DATDIR)/%.o, $(DATA_BG_FILES))
DATA_BG_INCLUDES	:= $(patsubst $(DATDIR)/%.bmp, $(BLDDIR)/$(DATDIR)/%.h, $(DATA_BG_FILES))

DATA_TL_FILES		:= $(wildcard $(DATDIR)/TL_*.bmp)
DATA_TL_SOURCES		:= $(patsubst $(DATDIR)/%.bmp, $(BLDDIR)/$(DATDIR)/%.c, $(DATA_TL_FILES))
DATA_TL_OBJECTS		:= $(patsubst $(DATDIR)/%.bmp, $(BLDDIR)/$(DATDIR)/%.o, $(DATA_TL_FILES))
DATA_TL_INCLUDES	:= $(patsubst $(DATDIR)/%.bmp, $(BLDDIR)/$(DATDIR)/%.h, $(DATA_TL_FILES))

DATA_SP_FILES		:= $(wildcard $(DATDIR)/SP_*.bmp)
DATA_SP_SOURCES		:= $(patsubst $(DATDIR)/%.bmp, $(BLDDIR)/$(DATDIR)/%.c, $(DATA_SP_FILES))
DATA_SP_OBJECTS		:= $(patsubst $(DATDIR)/%.bmp, $(BLDDIR)/$(DATDIR)/%.o, $(DATA_SP_FILES))
DATA_SP_INCLUDES	:= $(patsubst $(DATDIR)/%.bmp, $(BLDDIR)/$(DATDIR)/%.h, $(DATA_SP_FILES))

DATA_C_FILES		:= $(wildcard $(DATDIR)/*.c)
DATA_C_SOURCES		:= $(DATA_C_FILES)
DATA_C_OBJECTS		:= $(patsubst $(DATDIR)/%.c, $(BLDDIR)/$(DATDIR)/%.o, $(DATA_C_FILES))
DATA_C_INCLUDES		:= $(patsubst $(DATDIR)/%.c, $(INCDIR)/%.h, $(DATA_C_FILES))

DATA_FILES		:= $(DATA_BG_FILES) 	$(DATA_TL_FILES)	$(DATA_SP_FILES)
DATA_SOURCES	:= $(DATA_BG_SOURCES) 	$(DATA_TL_SOURCES)	$(DATA_SP_SOURCES)	$(DATA_C_SOURCES)
DATA_OBJECTS	:= $(DATA_BG_OBJECTS) 	$(DATA_TL_OBJECTS)	$(DATA_SP_OBJECTS)	$(DATA_C_OBJECTS)
DATA_INCLUDES	:= $(DATA_BG_INCLUDES) 	$(DATA_TL_INCLUDES)	$(DATA_SP_INCLUDES)	$(DATA_C_INCLUDES)










# Outros parametros para a compilação
INCLUDES	:= -I $(INCDIR) -I $(BLDDIR) -I $(DEVKITPRO)/libtonc/include
LIBRARIES	:= -L $(DEVKITPRO)/libtonc/lib -ltonc



# Target da compilação
TARGET	:= $(BINDIR)/$(NAME)
all: $(TARGET).gba


# Argumentos
CFLAGS		:= -Wall -mthumb -mthumb-interwork
LDFLAGS 	:= -Wall -mthumb -mthumb-interwork $(LIBRARIES) -specs=gba.specs



.PHONY: all clean clean_build clean_data clean_intermediates data data_objects debug dirs release










# Cores
RED		:= \033[0;31m#	Remover pastas
GREEN	:= \033[0;32m#	Criar pastas
YELLOW	:= \033[0;33m#	Convert imagens
BLUE	:= \033[0;34m#	Compilar fonte
PURPLE	:= \033[0;35m#	Linkagem e operações finais
CYAN	:= \033[0;36m#	Compilar arquivos de dados
WHITE	:= \033[0;37m#	Mensagem final
BRIGHT	:= \033[1m#		Brilho
RESET	:= \033[0m#		Resetar cores










# Converte as imagens BG_*.bmp em data/ do tipo
# mapa de peças (mode 0) para um arquivo .c e .h em build/data
$(BLDDIR)/$(DATDIR)/BG_%.c $(BLDDIR)/$(DATDIR)/BG_%.h: $(DATDIR)/BG_%.bmp | $(BUILD_DIRS)
	@echo "$(YELLOW)$(BRIGHT)BGS - $(YELLOW)$^ -> $@$(RESET)"
	@$(GRIT) $^ -mR8 -gB8 -mLs -gT FF00FF -ft c -o $@

# Converte as imagens TL_*.bmp em data/ do tipo
# peças (mode 0) para um arquivo .c e .h em build/data
$(BLDDIR)/$(DATDIR)/TL_%.c $(BLDDIR)/$(DATDIR)/TL_%.h: $(DATDIR)/TL_%.bmp | $(BUILD_DIRS)
	@echo "$(YELLOW)$(BRIGHT)TLS - $(YELLOW)$^ -> $@$(RESET)"
	@$(GRIT) $^ -m! -mR! -gB8 -gT FF00FF -ft c -o $@

# Converte as imagens SP_*.bmp em data/ do tipo
# sprite (mode 0) para um arquivo .c e .h em build/data
$(BLDDIR)/$(DATDIR)/SP_%.c $(BLDDIR)/$(DATDIR)/SP_%.h: $(DATDIR)/SP_%.bmp | $(BUILD_DIRS)
	@echo "$(YELLOW)$(BRIGHT)SPR - $(YELLOW)$^ -> $@$(RESET)"
	@$(GRIT) $^ -m! -mR! -gB4 -gT FF00FF -ft c -o $@










# Compila cada arquivo .c em build/data/ para .o em build/data/
$(BLDDIR)/$(DATDIR)/%.o: $(BLDDIR)/$(DATDIR)/%.c $(BLDDIR)/$(DATDIR)/%.h | $(BUILD_DIRS)
	@echo "$(CYAN)$(BRIGHT)BDI - $(CYAN)$< -> $@$(RESET)"
	@$(CC) $< $(CFLAGS) -c -o $@ $(INCLUDES)

# Compila cada arquivo .c em data/ para .o em build/data/
$(BLDDIR)/$(DATDIR)/%.o: $(DATDIR)/%.c $(INCDIR)/%.h | $(BUILD_DIRS)
	@echo "$(CYAN)$(BRIGHT)BDC - $(CYAN)$< -> $@$(RESET)"
	@$(CC) $< $(CFLAGS) -c -o $@ $(INCLUDES)










# Compila cada arquivo .c em source/ e .h em include/ para .o em build/
$(BLDDIR)/%.o: $(SRCDIR)/%.c $(INCDIR)/%.h | $(BUILD_DIRS)
	@echo "$(BLUE)$(BRIGHT)BSC - $(BLUE)$^ -> $@$(RESET)"
	@$(CC) $< $(CFLAGS) -c -o $@ $(INCLUDES)

# Compila cada arquivo .c em source/ para .o em build/
$(BLDDIR)/%.o: $(SRCDIR)/%.c | $(BUILD_DIRS)
	@echo "$(BLUE)$(BRIGHT)BSC - $(BLUE)$< -> $@$(RESET)"
	@$(CC) $< $(CFLAGS) -c -o $@ $(INCLUDES)










# Compila o binário .elf final
$(TARGET).elf: $(DATA_INCLUDES) $(DATA_OBJECTS) $(OBJECTS) | $(BUILD_DIRS)
	@echo "$(PURPLE)$(BRIGHT)ELF - $(PURPLE)'$(BLDDIR)/*.o $(BLDDIR)/$(DATDIR)/*.o' -> $@$(RESET)"
	@$(CC) $^ $(LDFLAGS) -o $@ $(INCLUDES)

# Compila o arquivo .gba final
$(TARGET).gba: $(TARGET).elf | $(BUILD_DIRS)
	@echo "$(PURPLE)$(BRIGHT)GBA - $(PURPLE)$^ -> $@"
	@echo -n "$(PURPLE)$(BRIGHT)CPY - $(PURPLE)"
	@$(OBJCOPY) -v -O binary $^ $@
	@echo -n "$(PURPLE)$(BRIGHT)FIX - $(PURPLE)"
	@$(GBAFIX) -t"$(NAME)" -m"$(MAKER)" -c"$(CODE)" "$@"
	@echo "$(WHITE)$(BRIGHT)END - $(WHITE)$@ compilado com sucesso!"
	@echo -n "$(RESET)"










# Cria as pastas necessárias para a compilação
$(BUILD_DIRS):
	@echo "$(GREEN)$(BRIGHT)MKD - $(GREEN)Criando '$@'$(RESET)"
	@mkdir -p $@

# Cria as pastas necessárias para a compilação
dirs: $(BUILD_DIRS)

# Cria os arquivos derivados das imagens
data: $(DATA_INCLUDES) $(DATA_SOURCES)
data_objects: $(DATA_OBJECTS)

# Faz a limpeza de tudo
clean:
	@echo "$(RED)$(BRIGHT)RMD - $(RED)Removendo '$(BUILD_DIRS)'$(RESET)"
	@rm -fr $(BUILD_DIRS)

# Faz a limpeza apenas dos arquivos de dados
clean_data:
	@echo "$(RED)$(BRIGHT)RMD - $(RED)Removendo '$(DATDIR)'$(RESET)"
	@rm -fr $(BLDDIR)/$(DATDIR)

# Faz a limpeza apenas dos arquivos de compilação
clean_build:
	@echo "$(RED)$(BRIGHT)RMD - $(RED)Removendo '$(BLDDIR)'$(RESET)"
	@rm -fr $(BLDDIR)

# Faz a limpeza apenas dos arquivos intermediários
clean_intermediates:
	@echo "$(RED)$(BRIGHT)RMD - $(RED)Removendo '$(BLDDIR) $(TARGET).elf'$(RESET)"
	@rm -fr $(BLDDIR) $(TARGET).elf

# Compilar com otimização
release: CFLAGS += -O3
release: all

# Compilar com informação de debug
debug: CFLAGS += -g3 -Og
debug: all
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
BMPCONV := $(DEVKITPRO)/tools/bin/grit



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

DATA_FILES		:= $(DATA_BG_FILES) 	$(DATA_TL_FILES)	$(DATA_SP_FILES)
DATA_SOURCES	:= $(DATA_BG_SOURCES) 	$(DATA_TL_SOURCES)	$(DATA_SP_SOURCES)
DATA_INCLUDES	:= $(DATA_BG_INCLUDES) 	$(DATA_TL_INCLUDES)	$(DATA_SP_INCLUDES)
DATA_OBJECTS	:= $(DATA_BG_OBJECTS) 	$(DATA_TL_OBJECTS)	$(DATA_SP_OBJECTS)










# Outros parametros para a compilação
INCLUDES	:= -I $(INCDIR) -I $(BLDDIR) -I $(DEVKITPRO)/libtonc/include
LIBRARIES	:= -L $(DEVKITPRO)/libtonc/lib -L $(DEVKITPRO)/libgba/lib -ltonc -lgba



# Target da compilação
TARGET	:= $(BINDIR)/$(NAME)
all: $(TARGET).gba
debug: all
debug: CFLAGS  += -g
debug: LDFLAGS += -g
.PHONY: all clean dirs data debug


# Argumentos
THUMB_ARGS 	:= -mthumb -mthumb-interwork
CFLAGS		:= -Wall $(THUMB_ARGS)
LDFLAGS 	:= $(LIBRARIES) -Wall $(THUMB_ARGS) -specs=gba.specs










# Converte as imagens BG_*.bmp em data/ do tipo
# mapa de peças (mode 0) para um arquivo .c e .h em build/data
$(BLDDIR)/$(DATDIR)/BG_%.c $(BLDDIR)/$(DATDIR)/BG_%.h: $(DATDIR)/BG_%.bmp | $(BUILD_DIRS)
	@echo "BGS - $^ -> $@"
	@$(BMPCONV) $^ -mR8 -gB8 -mLs -gT FF00FF -ft c -o $@

# Converte as imagens TL_*.bmp em data/ do tipo
# peças (mode 0) para um arquivo .c e .h em build/data
$(BLDDIR)/$(DATDIR)/TL_%.c $(BLDDIR)/$(DATDIR)/TL_%.h: $(DATDIR)/TL_%.bmp | $(BUILD_DIRS)
	@echo "TLS - $^ -> $@"
	@$(BMPCONV) $^ -m! -mR! -gB8 -gT FF00FF -ft c -o $@

# Converte as imagens SP_*.bmp em data/ do tipo
# sprite (mode 0) para um arquivo .c e .h em build/data
$(BLDDIR)/$(DATDIR)/SP_%.c $(BLDDIR)/$(DATDIR)/SP_%.h: $(DATDIR)/SP_%.bmp | $(BUILD_DIRS)
	@echo "SPR - $^ -> $@"
	@$(BMPCONV) $^ -m! -mR! -gB4 -gT FF00FF -ft c -o $@










# Compila cada arquivo .c em source/ para .o em build/
$(BLDDIR)/%.o: $(SRCDIR)/%.c | $(BUILD_DIRS)
	@echo "BSC - $^ -> $@"
	@$(CC) $^ $(CFLAGS) -c -o $@ $(INCLUDES)

# Compila cada arquivo .c em build/data/ para .o em build/data/
$(BLDDIR)/$(DATDIR)/%.o: $(BLDDIR)/$(DATDIR)/%.c | $(BUILD_DIRS)
	@echo "BDT - $^ -> $@"
	@$(CC) $^ $(CFLAGS) -c -o $@ $(INCLUDES)










# Compila o binário .elf final
$(TARGET).elf: $(DATA_INCLUDES) $(DATA_OBJECTS) $(OBJECTS)
	@echo "ELF - * -> $@"
	@$(CC) $^ $(LDFLAGS) -o $@ $(INCLUDES)

# Compila o arquivo .gba final
$(TARGET).gba: $(TARGET).elf
	@echo "GBA - $^ -> $@"
	@echo -n "CPY - "
	@$(OBJCOPY) -v -O binary $^ $@
	@echo -n "FIX - "
	@$(GBAFIX) -t"$(NAME)" -m"$(MAKER)" -c"$(CODE)" "$@"










# Cria as pastas necessárias para a compilação
$(BUILD_DIRS):
	@echo "MKD - Criando '$@'"
	@mkdir -p $@

# Cria as pastas necessárias para a compilação
dirs: $(BUILD_DIRS)

# Cria os arquivos derivados das imagens
# mas sem compilar os binários
data: $(DATA_INCLUDES) $(DATA_SOURCES)

# Faz a limpeza
clean:
	@echo "RMD - Removendo '$(BUILD_DIRS)'"
	@rm -fr $(BUILD_DIRS)
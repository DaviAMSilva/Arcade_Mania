# Editável
NAME	:= Arcade_Mania



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
BUILD_DIRS	:= $(BINDIR) $(BLDDIR) $(INCDIR)/$(DATDIR) $(BLDDIR)/$(DATDIR)



# Arquivos
SOURCES := $(wildcard $(SRCDIR)/*.c)
OBJECTS := $(patsubst $(SRCDIR)/%.c, $(BLDDIR)/%.o, $(SOURCES))

DATA_BG_FILES		:= $(wildcard $(DATDIR)/BG_*.bmp)
DATA_BG_SOURCES		:= $(patsubst $(DATDIR)/%.bmp, $(BLDDIR)/$(DATDIR)/%.c, $(DATA_BG_FILES))
DATA_BG_OBJECTS		:= $(patsubst $(DATDIR)/%.bmp, $(BLDDIR)/$(DATDIR)/%.o, $(DATA_BG_FILES))
DATA_BG_INCLUDES	:= $(patsubst $(DATDIR)/%.bmp, $(INCDIR)/$(DATDIR)/%.h, $(DATA_BG_FILES))

DATA_FILES		:= $(DATA_BG_FILES)
DATA_SOURCES	:= $(DATA_BG_SOURCES)
DATA_INCLUDES	:= $(DATA_BG_INCLUDES)
DATA_OBJECTS	:= $(DATA_BG_OBJECTS)

DATA_BG_PALETTE	:= $(BLDDIR)/$(DATDIR)/BG_MasterPalette
DATA_FILES		:= $(patsubst $(DATDIR)/%.bmp, $(BLDDIR)/$(DATDIR)/%, $(DATA_BG_FILES))


# Outas parametros para a compilação
INCLUDES	:= -I $(INCDIR) -I $(INCDIR)/$(DATDIR) -I $(DEVKITPRO)/libgba/include -I $(DEVKITPRO)/libtonc/include
LIBRARIES	:= $(DEVKITPRO)/libgba/lib/* $(DEVKITPRO)/libtonc/lib/*



# Target da compilação
TARGET	:= $(BINDIR)/$(NAME)
all: $(TARGET).gba



# Argumentos
THUMB_ARGS 		:= -mthumb -mthumb-interwork
DYNAMIC_ARGS 	:= -O2 -Wall $(THUMB_ARGS)
FINAL_ARGS 		:= -specs=gba.specs -O2 -Wall $(THUMB_ARGS)



.PHONY: all clean dirs data



# Converte as imagens .bmp em data/ do tipo
# background (mode 0) para um arquivo .c e .h em build/data
$(BLDDIR)/$(DATDIR)/BG_%.c $(BLDDIR)/$(DATDIR)/BG_%.h: $(DATDIR)/BG_%.bmp | $(BUILD_DIRS)
	@echo "BGS - $^ -> $@"
	@$(BMPCONV) $^ -gB8 -mRtpf -mLs -gT FF00FF -ft c -o $@



# Move cada arquivo .h em build/data/ para include/data/
$(INCDIR)/$(DATDIR)/%.h: $(BLDDIR)/$(DATDIR)/%.h | $(BUILD_DIRS)
	@echo "MOV - $(BLDDIR)/$(DATDIR)/*.h -> $(INCDIR)/$(DATDIR)"
	@mv $^ $(INCDIR)/$(DATDIR)

# Compila cada arquivo .c em source/ para .o em build/
$(BLDDIR)/%.o: $(SRCDIR)/%.c | $(BUILD_DIRS)
	@echo "C>O - $^ -> $@"
	@$(CC) $^ $(DYNAMIC_ARGS) -c -o $@ $(INCLUDES)

# Compila cada arquivo .c em build/data/ para .o em build/data/
$(BLDDIR)/$(DATDIR)/%.o: $(BLDDIR)/$(DATDIR)/%.c | $(BUILD_DIRS)
	@echo "C>O - $^ -> $@"
	@$(CC) $^ $(DYNAMIC_ARGS) -c -o $@ $(INCLUDES)



# Compila o binário .elf final
$(TARGET).elf: $(DATA_INCLUDES) $(DATA_OBJECTS) $(OBJECTS)
	@echo "ELF - $^ -> $@"
	@$(CC) $^ $(LIBRARIES) $(FINAL_ARGS) -o $@ $(INCLUDES)

# Compila o arquivo .gba final
$(TARGET).gba: $(TARGET).elf
	@echo "GBA - $^ -> $@"
	@echo -n "CPY - "
	@$(OBJCOPY) -v -O binary $^ $@
	@echo -n "FIX - "
	@$(GBAFIX) $@



# Cria as pastas necessárias para a compilação
$(BUILD_DIRS):
	@echo "MKD - Criando '$@'"
	@mkdir -p $@

# Cria as pastas necessárias para a compilação
dirs: $(BUILD_DIRS)

# Cria os arquivos derivados das imagens
# mas sem compilar os binários
data: $(DATA_INCLUDES)

# Faz a limpeza
clean:
	@echo "RMD - Removendo '$(BUILD_DIRS)'"
	@rm -fr $(BUILD_DIRS)
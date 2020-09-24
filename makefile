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
BUILD_DIRS	:= $(BINDIR) $(BLDDIR) $(BLDDIR)/$(DATDIR) $(INCDIR)/$(DATDIR)



# Arquivos
SOURCES := $(wildcard $(SRCDIR)/*.c)
OBJECTS := $(patsubst $(SRCDIR)/%.c, $(BLDDIR)/%.o, $(SOURCES))

DATA_FILES		:= $(wildcard $(DATDIR)/*.bmp)
# DATA_SOURCES	:= $(patsubst $(DATDIR)/%.bmp, $(BLDDIR)/$(DATDIR)/%.c, $(DATA_FILES))
DATA_OBJECTS	:= $(patsubst $(DATDIR)/%.bmp, $(BLDDIR)/$(DATDIR)/%.o, $(DATA_FILES))



# Target da compilação
TARGET	:= $(BINDIR)/$(NAME)
all: $(TARGET).gba



# Argumentos
THUMB_ARGS 		:= -mthumb -mthumb-interwork
DYNAMIC_ARGS 	:= -O2 -Wall $(THUMB_ARGS)
FINAL_ARGS 		:= -specs=gba.specs -O2 -Wall $(THUMB_ARGS)



.PHONY: all clean



# Compila cada arquivo .c em source para .o em build
$(BLDDIR)/%.o: $(SRCDIR)/%.c | $(BUILD_DIRS)
	@echo "$^ -> $@"
	@$(CC) $^ $(DYNAMIC_ARGS) -c -o $@

# TODO Pode ser excluido se o próximo target for alterado
# Compila cada arquivo .c em build/data para .o em build/data
#$(BLDDIR)/$(DATDIR)/%.o: $(BLDDIR)/$(DATDIR)/%.c
#	@echo "$^ -> $@"
#	@$(CC) $^ $(DYNAMIC_ARGS) -c -o $@

# Converte cada imagem .bmp em data para .c em build/data e .h em include/data
# $(BLDDIR)/$(DATDIR)/%.c $(INCDIR)/$(DATDIR)/%.h: $(DATDIR)/%.bmp
# TODO Usar grit
#	@echo "grit: $^ -> $@"
#	@echo > $(BLDDIR)/$(basename $^).c
#	@echo > $(INCDIR)/$(basename $^).h

# Compilar o binário final
$(TARGET).elf: $(OBJECTS)			#$(DATA_OBJECTS)
	@echo "$(BLDDIR)/*.o -> $@"
	@$(CC) $^ $(FINAL_ARGS) -o $@

# Compila o arquivo .gba final
$(TARGET).gba: $(TARGET).elf
	@echo "$^ -> $@"
	@$(OBJCOPY) -v -O binary $^ $@
	@$(GBAFIX) $@

# Cria as pastas necessárias para a compilação
$(BUILD_DIRS):
	@echo "Criando '$@'"
	@mkdir -p $@

# Faz a limpeza
clean:
	@echo "Removendo '$(BUILD_DIRS)'"
	@rm -fr $(BUILD_DIRS)
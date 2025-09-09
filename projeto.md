# Projeto Canary - OpenTibiaBR

## Visão Geral

O **Canary** é um servidor emulador de MMORPG gratuito e open-source escrito em C++. É um fork do projeto [OTServBR-Global](https://github.com/opentibiabr/otservbr-global) e representa uma das implementações mais avançadas de servidor Tibia disponíveis.

## Informações do Projeto

- **Nome**: OpenTibiaBR - Canary
- **Versão**: 3.1.2
- **Cliente Compatível**: 1340
- **Licença**: GNU General Public License v2
- **Linguagem Principal**: C++17/20
- **Sistema de Build**: CMake
- **Gerenciador de Dependências**: vcpkg

## Arquitetura Técnica

### Tecnologias Principais

- **C++17/20**: Linguagem principal do servidor
- **Lua**: Sistema de scripting para lógica do jogo
- **CMake**: Sistema de build multiplataforma
- **vcpkg**: Gerenciador de pacotes C++
- **MySQL/MariaDB**: Banco de dados para persistência
- **Docker**: Containerização para deployment
- **Protobuf**: Serialização de dados

### Dependências Principais

```json
{
	"dependencies": [
		"abseil",
		"argon2",
		"asio",
		"bext-di",
		"bext-ut",
		"curl",
		"eventpp",
		"libmariadb",
		"luajit",
		"magic-enum",
		"mio",
		"parallel-hashmap",
		"protobuf",
		"pugixml",
		"spdlog",
		"zlib",
		"bshoshany-thread-pool",
		"atomic-queue",
		"opentelemetry-cpp",
		"gmp",
		"mpir"
	]
}
```

## Estrutura do Projeto

### Código Fonte C++ (`src/`)

```
src/
├── account/          # Sistema de contas
├── config/           # Gerenciamento de configurações
├── creatures/        # Criaturas (players, monsters, NPCs)
├── database/         # Interface com banco de dados
├── enums/            # Definições de enums
├── game/             # Lógica principal do jogo
├── io/               # Operações de entrada/saída
├── items/            # Sistema de itens
├── kv/               # Sistema de chave-valor
├── lib/              # Bibliotecas auxiliares
├── lua/              # Integração Lua-C++
├── map/              # Sistema de mapas
├── protobuf/         # Definições Protocol Buffers
├── security/         # Segurança e criptografia
├── server/           # Servidor principal
└── utils/            # Utilitários
```

### Scripts Lua (`data-canary/`)

```
data-canary/
├── lib/              # Bibliotecas Lua
├── monster/          # Definições de monstros
├── npc/              # NPCs
├── raids/            # Sistema de raids
├── scripts/          # Scripts do jogo
│   ├── actions/      # Ações de itens
│   ├── creaturescripts/ # Eventos de criaturas
│   ├── globalevents/ # Eventos globais
│   ├── jutsus/       # Sistema de jutsus (Naruto)
│   ├── movements/    # Movimentos especiais
│   ├── spells/       # Magias
│   ├── systems/      # Sistemas customizados
│   └── talkactions/  # Comandos de chat
└── world/            # Mapas e configurações do mundo
```

## Funcionalidades Principais

### Sistema de Jogo

- **PvP/PvE**: Suporte completo para combate
- **Sistema de Classes**: Vocações com habilidades únicas
- **Sistema de Níveis**: Progressão com stages configuráveis
- **Sistema de Itens**: Equipamentos, armas, poções, runas
- **Sistema de Magias**: Spells e runas mágicas
- **Sistema de Casas**: Propriedades com aluguel
- **Sistema de Guilds**: Organizações de jogadores

### Sistemas Avançados

- **Imbuement**: Encantamentos de itens
- **Prey System**: Sistema de presas
- **Task Hunting**: Sistema de missões
- **Forge System**: Sistema de forja
- **Bestiary**: Enciclopédia de criaturas
- **Wheel of Destiny**: Sistema de habilidades
- **Familiar System**: Sistema de companheiros
- **VIP System**: Sistema premium

### Sistemas Customizados (Naruto)

- **Sistema de Jutsus**: Técnicas especiais por clã
- **Clãs**: Uchiha, Inuzuka, Hyuuga, etc.
- **Summons**: Invocações especiais
- **Outfits**: Roupas temáticas de Naruto

## Configuração

### Arquivo de Configuração (`config.lua`)

O servidor é altamente configurável através do arquivo `config.lua`:

```lua
-- Configurações principais
dataPackDirectory = "data-canary"
serverName = "Arise"
serverMotd = "Welcome to the Shinobi Arise!"

-- Configurações de combate
worldType = "pvp"
protectionLevel = 8
experienceByKillingPlayers = false

-- Configurações de conexão
ip = "127.0.0.1"
loginProtocolPort = 7171
gameProtocolPort = 7172
maxPlayers = 0

-- Configurações de banco de dados
mysqlHost = "127.0.0.1"
mysqlUser = "root"
mysqlPass = "root"
mysqlDatabase = "canary"
mysqlPort = 3306

-- Rates configuráveis
rateExp = 1
rateSkill = 1
rateLoot = 1
rateMagic = 1
rateSpawn = 1
```

### Sistema de Stages (`data/stages.lua`)

```lua
experienceStages = {
    {minlevel = 1, maxlevel = 8, multiplier = 7},
    {minlevel = 9, maxlevel = 20, multiplier = 6},
    {minlevel = 21, maxlevel = 50, multiplier = 5},
    {minlevel = 51, maxlevel = 100, multiplier = 4},
    {minlevel = 101, multiplier = 2}
}
```

## Sistema de Scripting

### RevScripts

O Canary utiliza o sistema **RevScripts**, que permite:

- **Carregamento dinâmico**: Scripts podem ser recarregados sem reiniciar o servidor
- **Sintaxe moderna**: Uso de closures e funções anônimas
- **Melhor performance**: Otimizações internas do sistema

### Exemplo de RevScript (Imbuement Shrine)

```lua
local imbuement = Action()

function imbuement.onUse(player, item, fromPosition, target, toPosition, isHotkey)
    if configManager.getBoolean(configKeys.TOGGLE_IMBUEMENT_SHRINE_STORAGE) and
       player:getStorageValue(Storage.Imbuement) ~= 1 then
        return player:sendTextMessage(MESSAGE_EVENT_ADVANCE,
            "You did not collect enough knowledge from the ancient Shapers.")
    end

    if not target or type(target) ~= "userdata" or not target:isItem() then
        return player:sendTextMessage(MESSAGE_EVENT_ADVANCE,
            "You can only use the shrine on an valid item.")
    end

    player:openImbuementWindow(target)
    return true
end

imbuement:id(25060, 25061, 25103, 25104, 25202, 25174, 25175, 25182, 25183)
imbuement:register()
```

## Banco de Dados

### Estrutura Principal

- **accounts**: Contas de jogadores
- **players**: Dados dos personagens
- **guilds**: Informações de guilds
- **houses**: Propriedades
- **market_offers**: Sistema de mercado
- **player_deaths**: Histórico de mortes
- **player_storage**: Sistema de storage

### Schema SQL

```sql
CREATE TABLE IF NOT EXISTS `accounts` (
    `id` int(11) UNSIGNED NOT NULL AUTO_INCREMENT,
    `name` varchar(32) NOT NULL,
    `password` TEXT NOT NULL,
    `email` varchar(255) NOT NULL DEFAULT '',
    `premdays` int(11) NOT NULL DEFAULT '0',
    `coins` int(12) UNSIGNED NOT NULL DEFAULT '0',
    `creation` int(11) UNSIGNED NOT NULL DEFAULT '0',
    CONSTRAINT `accounts_pk` PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
```

## Deployment

### Docker

O projeto inclui configuração Docker completa:

```yaml
version: "3.8"
services:
  database:
    image: mariadb:10.6
    environment:
      MYSQL_ROOT_PASSWORD: root
      MYSQL_DATABASE: canary
    volumes:
      - ./docker/data:/docker-entrypoint-initdb.d

  server:
    build:
      context: .
      dockerfile: docker/Dockerfile.dev
    depends_on:
      - database
    ports:
      - "7171:7171"
      - "7172:7172"
```

### Build Manual

```bash
# Instalar dependências
cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake ..

# Compilar
make -j$(nproc)

# Executar
./canary
```

## Características Especiais

### Sistema de Segurança

- **Argon2**: Hashing seguro de senhas
- **RSA**: Criptografia para comunicação
- **Proteção contra WPE**: Limitação de pacotes por segundo

### Performance

- **Multithreading**: Suporte a múltiplos threads
- **Pool de Conexões**: Gerenciamento eficiente de conexões
- **Cache**: Sistema de cache para otimização
- **Compressão**: Compressão de pacotes de rede

### Monitoramento

- **Prometheus**: Métricas de sistema
- **Logging**: Sistema de logs avançado
- **Discord Webhooks**: Notificações automáticas

## Comunidade e Suporte

- **Discord**: https://discord.gg/gvTj5sh9Mp
- **Documentação**: https://docs.opentibiabr.com/
- **GitHub**: https://github.com/opentibiabr/canary
- **Wiki**: https://github.com/opentibiabr/canary/wiki

## Contribuição

O projeto aceita contribuições através de:

- **Issue Tracker**: Para reportar bugs e sugerir features
- **Pull Requests**: Para contribuições de código
- **Documentação**: Melhorias na documentação
- **Testes**: Adição de testes unitários e de integração

## Licenciamento

Este projeto está licenciado sob a **GNU General Public License v2**, garantindo:

- Liberdade de uso
- Liberdade de modificação
- Liberdade de distribuição
- Liberdade de melhorias

## Conclusão

O Canary representa o estado da arte em servidores emuladores de Tibia, oferecendo:

- **Arquitetura moderna** com C++17/20
- **Sistema de scripting flexível** com Lua
- **Alta performance** e escalabilidade
- **Comunidade ativa** e suporte contínuo
- **Funcionalidades avançadas** e sistemas customizados
- **Documentação completa** e ferramentas de desenvolvimento

É uma excelente escolha para desenvolvedores que desejam criar servidores MMORPG robustos e personalizáveis.

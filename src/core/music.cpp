#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>

// Music controller implementation
// SxMML (S3HS Extended Music Macro Language, MML方言)

#define SOUND_REGISTER_OFFSET 0x400000
#define SOUND_REGISTER_SIZE 0x400

// SxMML Compiler, converts SxMML data into sound register commands
class SxMMLCompiler
{
public:
    // Constructor
    SxMMLCompiler(const char *mmlData) {

    };
    // Compiles SxMML data into sound register commands (returns: Error string or "OK")
    std::string compile(std::string mmlData)
    {
        // Placeholder implementation
        return "NOT_IMPLEMENTED";
    }
};

// Primitive driver, Reads register commands and writes to sound registers in proper timing
class MusicDriver
{
public:
    MusicDriver(uint8_t *soundRegisters)
        : soundRegisters(soundRegisters), isPlaying(false), tickCounter(0) {}

    // Starts music playback from the beginning
    void play(const uint8_t *musicData, size_t dataSize)
    {
        this->musicData = musicData;
        this->dataSize = dataSize;
        this->tickCounter = 0;
        this->waitTicks = 0;
        this->loopCounter = 0;
        this->dataPointer = 0;
        this->isPlaying = true;
    }

    // Stops music playback
    void stop()
    {
        this->isPlaying = false;
    }

    // Opcodes
    enum registerCommands
    {                                             // Operation, (operands)
        CMD_NOOP = 0x00,                          // No operation ()
        CMD_WRITE_REG = 0x01,                     // Write to sound register (addr_hi, addr_lo, value)
        CMD_WAIT = 0x02,                          // Wait for specified number of ticks (tick_count_hi, tick_count_lo)
        CMD_JUMP = 0x03,                          // Jump to position (cmd_addr_hi, cmd_addr_lo)
        CMD_JUMP_IF_LOOP_COUNTER_NOT_ZERO = 0x04, // Jump if loop counter not zero (also decrements loop counter) (cmd_addr_hi, cmd_addr_lo)
        CMD_SET_LOOP_COUNTER = 0x05,              // Set loop counter (count)
        CMD_SET_TICK_RATE = 0x06,                 // Set tick rate in Hz (ticks_per_second)
        CMD_END = 0xFF                            // End of music data ()
    };

    // Updates the music driver (should be called every tick)
    void update()
    {
        if (waitTicks > 0)
        {   // do nothing until waitTicks reaches 0
            waitTicks--;
            return;
        }
        if (!isPlaying)
            return; // not playing

        // runs continuously until waitTicks is set
        while (waitTicks > 0)
        {
            uint8_t command = musicData[dataPointer];
            switch (command)
            {
            case CMD_NOOP:
                dataPointer += 1;
                break;
            case CMD_WRITE_REG:
            {
                if (dataPointer + 3 > dataSize)
                {
                    printf("Error in SoundDriver: Out of bounds on WRITE_REG\n");
                    isPlaying = false; // Error: Out of bounds
                    return;
                }
                uint16_t addr = (musicData[dataPointer] << 8) | musicData[dataPointer + 1];
                uint8_t value = musicData[dataPointer + 2];
                if (addr >= SOUND_REGISTER_OFFSET && addr < SOUND_REGISTER_OFFSET + SOUND_REGISTER_SIZE)
                {
                    // TODO: Write to sound register
                }
                dataPointer += 3;
                break;
            }
            default:
                printf("Error in SoundDriver: Unknown command: 0x%02X\n", command);
                dataPointer += 1;
                break;
            }
        }
    }

private:
    uint8_t *soundRegisters;
    const uint8_t *musicData;
    size_t dataSize;
    size_t dataPointer;
    bool isPlaying;
    int tickCounter;
    int waitTicks;
    int loopCounter;
};
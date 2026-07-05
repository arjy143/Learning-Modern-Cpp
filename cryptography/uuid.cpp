#include <array>
#include <cstdint>
#include <string>

class Uuid
{
private:

    std::array<uint8_t, 16> bytes_;

public:

    constexpr Uuid() noexcept = default;

    constexpr explicit Uuid(const std::array<uint8_t, 16>& bytes) noexcept : bytes_{bytes} {}

    [[nodiscard]] std::string to_string() const
    {
        static constexpr char hex[] = "0123456789abcdef";

        std::string s(36, '-');

        std::size_t pos = 0;

        for (const std::uint8_t b : bytes_)
        {
            if (pos == 8 || pos == 13 || pos == 18 || pos == 23)
            {
                ++pos;
            }

            s[pos++] = hex[b >> 4];
            s[pos++] = hex[b & 0x0f];
        }

        return s;
    }

    [[nodiscard]] constexpr int version() const noexcept 
    {
        //return high nibble of byte 6
        return bytes_[6] >> 4;
    }

    friend constexpr bool operator==(const Uuid&, const Uuid&) noexcept = default;

};

static_assert(sizeof(Uuid) == 16);

[[nodiscard]] Uuid generate_uuid() 
{

    thread_local std::random_device rd;
    const uint32_t a = rd();
    const uint32_t b = rd();
    const uint32_t c = rd();
    const uint32_t d = rd();
    

    std::array<uint8_t, 16> bytes;

    std::array<int, 4> source{a,b,c,d};

    memcpy(bytes.data(), source.data(), sizeof(source));

    bytes[6] = static_cast<std::uint8_t>((bytes[6] & 0x0f) | 0x40);
    bytes[8] = static_cast<std::uint8_t>((bytes[8] & 0x3f) | 0x80);

    Uuid uuid(bytes);

    return uuid;
}
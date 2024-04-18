#include <algorithm>
#include <cstring>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <unordered_map>
#include <type_traits>

enum class FileType {
    PNG,
    MP4,
    WAV,
    TXT,
    UNKNOWN
};

struct PNG;
struct MP4;
struct WAV;
struct TXT;
struct UNKNOWN;

// SIGNATURE
constexpr uint8_t PNGSIGNATURE[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
constexpr char WAVSIGNATURE[] = {0x52, 0x49, 0x46, 0x46};

// PNG HEADER STRUCTURE
struct PNG_IHDR {
    char width[4];
    char height[4];
    char bitDepth;
    char colourType;
    char compressionMethod;
    char filterMethod;
    char interlaceMethod;
};

struct MP4_FTYP {
    char majorBrand[4];
    char minorVersion[4];
    char compatibleBrands[8];
};

// WAV HEADER STRUCTURE
struct WAV_Metadata {
    char channels[2];
    char sampleRate[4];
    char bitRate[4];
    char blocksAlign[2];
    char bitDepth[2];
};

// CONCEPT
template<typename T>
concept SUPPORTED_FORMAT = std::same_as<T, PNG> or
                     std::same_as<T, WAV> or
                     std::same_as<T, MP4> or
                     std::same_as<T, TXT> or
                     std::same_as<T, UNKNOWN>;

template<typename T>
std::unordered_map<std::string, std::string> analyzeBasic(const std::filesystem::path &filepath) {
    std::unordered_map<std::string, std::string> basicMetadata;

    basicMetadata["File Name"] = filepath.filename();
    basicMetadata["File Size"] = std::to_string(std::filesystem::file_size(filepath));
    if (std::is_same<T, PNG>::value == true) {
        basicMetadata["Format"] = "PNG";
    } else if (std::is_same<T, WAV>::value == true) {
        basicMetadata["Format"] = "WAV";
    } else {
        basicMetadata["Format"] = "UNKNOWN";
    }

    return basicMetadata;
};

template<SUPPORTED_FORMAT T>
class FileMetadataAnalyzer {
private:
    std::unordered_map<std::string, std::string> metadata;

public:
    void analyze(const std::filesystem::path &filepath) {
        metadata = analyzeBasic<T>(filepath);
    }

    std::unordered_map<std::string, std::string> getMetadata() {
        return metadata;
    }
};

template<>
class FileMetadataAnalyzer<PNG> {
private:
    std::unordered_map<std::string, std::string> metadata;

public:
    void analyze(const std::filesystem::path &filepath) {
        metadata = analyzeBasic<PNG>(filepath);

        std::ifstream file(filepath, std::ios::binary);
        file.seekg(16, std::ios::beg);

        PNG_IHDR header{};
        file.read(reinterpret_cast<char *>(&header), sizeof(header));
        file.close();

        // Converting Little Endian to Big Endian
        std::reverse(header.width, header.width + sizeof(header.width));
        std::reverse(header.height, header.height + sizeof(header.height));

        // Casting to String
        std::string width = std::to_string(*reinterpret_cast<uint32_t *>(header.width));
        std::string height = std::to_string(*reinterpret_cast<uint32_t *>(header.height));
        std::string bitDepth = std::to_string(header.bitDepth);
        std::string colourType = std::to_string(header.colourType);
        std::string compressionMethod = std::to_string(header.compressionMethod);
        std::string filterMethod = std::to_string(header.filterMethod);
        std::string interlaceMethod = std::to_string(header.interlaceMethod);


        // Setting Metadata Fields
        this->metadata["Width"] = width;
        this->metadata["Height"] = height;
        this->metadata["Bit Depth"] = bitDepth;
        this->metadata["Colour Type"] = colourType;
        this->metadata["Compression Method"] = compressionMethod;
        this->metadata["Filter Method"] = filterMethod;
        this->metadata["Interlace Method"] = interlaceMethod;
    }

    std::unordered_map<std::string, std::string> getMetadata() {
        return metadata;
    }
};

template<>
class FileMetadataAnalyzer<WAV> {
private:
    std::unordered_map<std::string, std::string> metadata;

public:
    void analyze(const std::filesystem::path &filepath) {
        metadata = analyzeBasic<WAV>(filepath);

        std::ifstream file(filepath, std::ios::binary);
        file.seekg(22, std::ios::beg);

        WAV_Metadata header{};
        file.read(reinterpret_cast<char *>(&header), sizeof(header));
        file.close();

        // Casting to String
        std::string channels = std::to_string(*reinterpret_cast<uint16_t *>(header.channels));
        std::string sampleRate = std::to_string(*reinterpret_cast<uint32_t *>(header.sampleRate));
        std::string bitRate = std::to_string(*reinterpret_cast<uint32_t *>(header.bitRate));
        std::string bitDepth = std::to_string(*reinterpret_cast<uint16_t *>(header.bitDepth));


        // Setting Metadata Fields
        this->metadata["Channels"] = channels;
        this->metadata["Sample Rate"] = sampleRate;
        this->metadata["Bit Rate"] = bitRate;
        this->metadata["Bit Depth"] = bitDepth;
    }

    std::unordered_map<std::string, std::string> getMetadata() {
        return metadata;
    }
};

// Get File Type

FileType getFileType(const std::filesystem::path &filepath) {
    std::ifstream file(filepath, std::ios::binary);

    uint8_t signature[8];

    file.read(reinterpret_cast<char *>(&signature), sizeof(signature));

    file.close();

    if (!memcmp(PNGSIGNATURE, signature, sizeof(PNGSIGNATURE))) {
        return FileType::PNG;
    } else if (!memcmp(WAVSIGNATURE, signature, sizeof(WAVSIGNATURE))) {
        return FileType::WAV;
    } else {
        return FileType::UNKNOWN;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: FileMetadataAnalyzer <filepath>" << std::endl;
        exit(1);
    }

    std::string filepath_string = argv[1];
    std::filesystem::path filepath = std::filesystem::path(filepath_string);

    bool fileExists = std::filesystem::exists(filepath);

    if (!fileExists) {
        std::cerr << "Could not find file" << std::endl;
        exit(1);
    }

    FileType fileType = getFileType(filepath);
    std::unordered_map<std::string, std::string> metadata;

    if (fileType == FileType::PNG) {
        FileMetadataAnalyzer<PNG> file_metadata_analyzer;
        file_metadata_analyzer.analyze(filepath);
        metadata = file_metadata_analyzer.getMetadata();
    } else if (fileType == FileType::WAV) {
        FileMetadataAnalyzer<WAV> file_metadata_analyzer;
        file_metadata_analyzer.analyze(filepath);
        metadata = file_metadata_analyzer.getMetadata();
    } else {
        FileMetadataAnalyzer<UNKNOWN> file_metadata_analyzer;
        file_metadata_analyzer.analyze(filepath);
        metadata = file_metadata_analyzer.getMetadata();
    }

    auto display = []<typename K, typename V>(K key, V value) {
        std::cout << key << " : " << value << std::endl;
    };

    std::cout << "----------METADATA----------" << std::endl;
    for (const auto &[key, value]: metadata) {
        display(key, value);
    }
    std::cout << "----------------------------" << std::endl;

    return 0;
}

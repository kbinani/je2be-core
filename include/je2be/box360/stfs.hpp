/*
 * This derived work is from the Velocity project licensed under GNU General Public License (v3).
 * The original author of the Velocity is hetelek https://github.com/hetelek at https://github.com/hetelek/Velocity.
 * Fork of the Velocity by Gualdimar https://github.com/Gualdimar at https://github.com/Gualdimar/Velocity.
 */
#pragma once

namespace je2be::box360 {

using INT24 = signed int;

typedef struct _WINFILETIME {
  uint32_t dwHighDateTime;
  uint32_t dwLowDateTime;

} WINFILETIME;

#pragma region StfConstants.h

#define INT24_MAX 0xFFFFFF

enum Sex {
  StfsFemale = 0,
  StfsMale
};

enum Level {
  Zero,
  One,
  Two
};

enum ConsoleType {
  DevKit = 1,
  Retail = 2
};

enum ConsoleTypeFlags {
  TestKit = 0x40000000,
  RecoveryGenerated = 0x80000000
};

enum Magic {
  CON = 0x434F4E20,
  LIVE = 0x4C495645,
  PIRS = 0x50495253
};

enum InstallerType {
  None = 0,
  SystemUpdate = 0x53555044,
  TitleUpdate = 0x54555044,
  SystemUpdateProgressCache = 0x50245355,
  TitleUpdateProgressCache = 0x50245455,
  TitleContentProgressCache = 0x50245443
};

enum ContentType {
  ArcadeGame = 0xD0000,
  AvatarAssetPack = 0x8000,
  AvatarItem = 0x9000,
  CacheFile = 0x40000,
  CommunityGame = 0x2000000,
  GameDemo = 0x80000,
  GameOnDemand = 0x7000,
  GamerPicture = 0x20000,
  GamerTitle = 0xA0000,
  GameTrailer = 0xC0000,
  GameVideo = 0x400000,
  InstalledGame = 0x4000,
  Installer = 0xB0000,
  IPTVPauseBuffer = 0x2000,
  LicenseStore = 0xF0000,
  MarketPlaceContent = 2,
  Movie = 0x100000,
  MusicVideo = 0x300000,
  PodcastVideo = 0x500000,
  Profile = 0x10000,
  Publisher = 3,
  SavedGame = 1,
  StorageDownload = 0x50000,
  Theme = 0x30000,
  Video = 0x200000,
  ViralVideo = 0x600000,
  XboxDownload = 0x70000,
  XboxOriginalGame = 0x5000,
  XboxSavedGame = 0x60000,
  Xbox360Title = 0x1000,
  IndieGame = 0xE0000
};

struct Version {
  uint16_t major, minor, build, revision;
};
#pragma endregion

#pragma region IO_BaseIO.h

enum EndianType {
  BigEndian,
  LittleEndian,
  Default
};

class BaseIO {
public:
  BaseIO() : byteOrder(BigEndian) {
    // should be implemented by derived class
  }

  virtual ~BaseIO() {}

  // set the byte order in which to read the bytes
  void SetEndian(EndianType byteOrder) {
    this->byteOrder = byteOrder;
  }

  // swap the endianess
  void SwapEndian() {
    if (byteOrder == LittleEndian)
      byteOrder = BigEndian;
    else
      byteOrder = LittleEndian;
  }

  // seek to a position in a file
  virtual void SetPosition(uint64_t position, std::ios_base::seekdir dir = std::ios_base::beg) = 0;

  // get current address in the file
  virtual uint64_t GetPosition() = 0;

  // get length of the file
  virtual uint64_t Length() = 0;

  // get the byte order in which to read the bytes
  EndianType GetEndian() {
    return this->byteOrder;
  }

  // read 'len' bytes from the current file at the current position into buffer
  virtual void ReadBytes(uint8_t *outBuffer, uint32_t len) = 0;

  // write 'len' bytes from the current file at the current position into buffer
  virtual void WriteBytes(uint8_t *buffer, uint32_t len) = 0;

  // resize the current object (if supported)
  virtual void Resize(uint64_t size) { throw std::string("Resizing not supported."); }

  // all the read functions
  uint8_t ReadByte() {
    uint8_t toReturn;
    ReadBytes(&toReturn, 1);
    return toReturn;
  }

  int16_t ReadInt16() {
    return (int16_t)ReadWord();
  }

  uint16_t ReadWord() {
    uint16_t toReturn;
    ReadBytes(reinterpret_cast<uint8_t *>(&toReturn), 2);

    if (byteOrder == BigEndian)
      reverseByteArray(reinterpret_cast<uint8_t *>(&toReturn), 2);

    return toReturn;
  }

  INT24 ReadInt24(EndianType et = Default) {
    EndianType orig = byteOrder;

    if (et != Default)
      byteOrder = et;

    INT24 returnVal = ReadDword();

    if (byteOrder == BigEndian)
      returnVal = (returnVal & 0xFFFFFF00) >> 8;
    else
      returnVal = returnVal & 0x00FFFFFF;

    SetPosition(GetPosition() - 1);
    byteOrder = orig;

    return returnVal;
  }

  int32_t ReadInt32() {
    return (int32_t)ReadDword();
  }

  uint32_t ReadDword() {
    uint32_t toReturn;
    ReadBytes(reinterpret_cast<uint8_t *>(&toReturn), 4);

    if (byteOrder == BigEndian)
      reverseByteArray(reinterpret_cast<uint8_t *>(&toReturn), 4);

    return toReturn;
  }

  int64_t ReadInt64() {
    return (int64_t)ReadUInt64();
  }

  uint64_t ReadUInt64() {
    uint64_t toReturn;
    ReadBytes(reinterpret_cast<uint8_t *>(&toReturn), 8);

    if (byteOrder == BigEndian)
      reverseByteArray(reinterpret_cast<uint8_t *>(&toReturn), 8);

    return toReturn;
  }

  uint64_t ReadMultiByte(size_t size) {
    switch (size) {
    case 1:
      return ReadByte();
    case 2:
      return ReadWord();
    case 4:
      return ReadDword();
    case 8:
      return ReadUInt64();
    default:
      throw std::string("BaseIO: Invalid multi-byte size.\n");
    }
  }

  float ReadFloat() {
    uint32_t temp = ReadDword();
    return *(float *)&temp;
  }

  double ReadDouble() {
    uint64_t temp = ReadUInt64();
    return *(double *)&temp;
  }

  std::string ReadString(int len = -1, char nullTerminator = 0, bool forceInclude0 = true, int maxLength = 0x7FFFFFFF) {
    std::string toReturn;

    // assume it's null terminating
    if (len == -1) {
      toReturn = "";
      int i = 1;
      char nextChar;
      while ((nextChar = ReadByte()) != nullTerminator && (forceInclude0 && nextChar != 0) && (i++ <= maxLength))
        toReturn += nextChar;
    } else {
      std::vector<char> strVec;
      strVec.reserve(len + 1);
      char *str = strVec.data();
      str[len] = 0;

      ReadBytes((uint8_t *)str, len);
      toReturn = std::string(str);
    }

    return toReturn;
  }

  std::wstring ReadWString(int len = -1) {
    std::wstring toReturn;

    // assume it's null terminating
    if (len == -1) {
      toReturn = L"";
      wchar_t nextChar;
      while ((nextChar = ReadWord()) != 0)
        toReturn += nextChar;
    } else {
      for (int i = 0; i < len; i++) {
        wchar_t c = static_cast<wchar_t>(ReadWord());
        if (c != 0)
          toReturn += c;
        else
          break;
      }
    }

    return toReturn;
  }

  // Write functions
  void Write(uint8_t b) {
    WriteBytes(&b, 1);
  }

  void Write(uint16_t w) {
    if (byteOrder == BigEndian)
      reverseByteArray(reinterpret_cast<uint8_t *>(&w), 2);
    WriteBytes(reinterpret_cast<uint8_t *>(&w), 2);
  }

  void Write(INT24 i24, EndianType et = Default) {
    EndianType orig = byteOrder;
    if (et != Default)
      byteOrder = et;

    if (byteOrder == BigEndian) {
      i24 <<= 8;
      reverseByteArray(reinterpret_cast<uint8_t *>(&i24), 4);
    }
    WriteBytes(reinterpret_cast<uint8_t *>(&i24), 3);

    byteOrder = orig;
  }

  void Write(uint32_t dw) {
    if (byteOrder == BigEndian)
      reverseByteArray(reinterpret_cast<uint8_t *>(&dw), 4);
    WriteBytes(reinterpret_cast<uint8_t *>(&dw), 4);
  }

  void Write(uint64_t u64) {
    if (byteOrder == BigEndian)
      reverseByteArray(reinterpret_cast<uint8_t *>(&u64), 8);
    WriteBytes(reinterpret_cast<uint8_t *>(&u64), 8);
  }

  void Write(std::string s, int forceLen = -1, bool nullTerminating = true, uint8_t nullTerminator = 0) {
    WriteBytes((uint8_t *)s.c_str(), s.length() + nullTerminating);

    if (forceLen > 0) {
      forceLen -= s.size();

      for (int i = 0; i < forceLen; i++)
        Write(nullTerminator);
    }
  }

  void Write(std::wstring ws, bool nullTerminating = true) {
    if (byteOrder == LittleEndian)
      WriteBytes((uint8_t *)ws.c_str(), (ws.length() + nullTerminating) * 2);
    else {
      uint16_t curChar;
      for (uint32_t i = 0; i < ws.length(); i++) {
        curChar = ws.at(i);
        Write(curChar);
      }
      if (nullTerminating)
        Write((uint16_t)0);
    }
  }

  void Write(uint8_t *buffer, uint32_t len) {
    WriteBytes(buffer, len);
  }

  // flush the io's buffer
  virtual void Flush() = 0;

  // close the io
  virtual void Close() = 0;

protected:
  EndianType byteOrder;

private:
  void reverseByteArray(uint8_t *array, uint32_t len) {
    uint8_t temp;
    for (uint32_t i = 0; i < len / 2; i++) {
      temp = array[i];
      array[i] = array[len - i - 1];
      array[len - i - 1] = temp;
    }
  }
};
#pragma endregion

#pragma region IO_FileIO.h
class FileIO : public BaseIO {
public:
  FileIO(std::string path, bool truncate = false) : BaseIO(), filePath(path) {
    using namespace std;
    fstr = new fstream(path.c_str(), fstream::in | fstream::out | fstream::binary | (truncate ? fstream::trunc : static_cast<std::ios_base::openmode>(0)));
    if (!fstr->is_open()) {
      std::string ex("FileIO: Error opening the file. ");
      ex += strerror(errno);
      ex += "\n";
      throw ex;
    }

    endian = BigEndian;

    fstr->rdbuf()->pubsetbuf(0, 0);
    fstr->seekp(0);
  }

  void SetPosition(uint64_t pos, std::ios_base::seekdir dir = std::ios_base::beg) {
    fstr->seekp(pos, dir);
  }

  uint64_t GetPosition() {
    return fstr->tellp();
  }

  uint64_t Length() {
    uint64_t originalPosition = GetPosition();

    fstr->seekp(0, std::ios_base::end);
    uint64_t fileLength = fstr->tellp();

    SetPosition(originalPosition);
    return fileLength;
  }

  void ReadBytes(uint8_t *outBuffer, uint32_t len) {
    fstr->read((char *)outBuffer, len);
    if (fstr->fail())
      throw std::string("FileIO: Error reading from file.\n") + std::string(strerror(errno));
  }

  void WriteBytes(uint8_t *buffer, uint32_t len) {
    fstr->write((char *)buffer, len);
    if (fstr->fail())
      throw std::string("FileIO: Error writing to file.\n") + std::string(strerror(errno));
  }

  void Close() {
    fstr->close();
  }

  void Flush() {
    fstr->flush();
  }

  void Resize(uint64_t size) {
    using namespace std;
    if (size > this->Length())
      throw std::string("FileIO: Cannot expand file size.");

    uint8_t *buffer = new uint8_t[0x10000];
    std::string newFilePath = filePath + ".new";

    // open a new stream
    fstream *newFileStream = new fstream(newFilePath.c_str(), ios::out | ios::binary | ios::trunc);
    if (!fstr->is_open()) {
      std::string ex("FileIO: Failed to resize file. ");
      ex += strerror(errno);
      ex += "\n";
      throw ex;
    }

    // copy the data
    while (size >= 0x10000) {
      fstr->read((char *)buffer, 0x10000);
      newFileStream->write((char *)buffer, 0x10000);
      size -= 0x10000;
    }

    if (size != 0) {
      fstr->read((char *)buffer, size);
      newFileStream->write((char *)buffer, size);
    }
    delete[] buffer;

    // close the current stream, delete the files
    fstr->close();
    remove(filePath.c_str());

    newFileStream->close();
    delete newFileStream;

    // rename the new file to the correct original name
    if (rename(newFilePath.c_str(), filePath.c_str()) != 0)
      throw std::string("FileIO: Failed to resize file.");

    // set the final stream
    fstr = new fstream(filePath.c_str(), ios::in | ios::out | ios::binary);
  }

  std::string GetFilePath() {
    return filePath;
  }

  virtual ~FileIO() {
    if (fstr->is_open())
      fstr->close();
    delete fstr;
  }

private:
  EndianType endian;
  std::fstream *fstr;
  std::string filePath;
};
#pragma endregion

class MemoryIO : public BaseIO {
public:
  void SetPosition(uint64_t position, std::ios_base::seekdir dir = std::ios_base::beg) override {
    uint64_t pos;
    switch (dir) {
    case std::ios_base::cur:
      pos = this->position + position;
      break;
    case std::ios_base::end:
      pos = buffer.size() + position;
      break;
    case std::ios_base::beg:
    default:
      pos = position;
      break;
    }
    if (this->position < pos) {
      buffer.resize(pos);
    }
    this->position = pos;
  }

  uint64_t GetPosition() override {
    return position;
  }

  uint64_t Length() override {
    return buffer.size();
  }

  void ReadBytes(uint8_t *outBuffer, uint32_t len) override {
    if (buffer.size() <= position + len) {
      throw std::string("MemoryIO: index out of range");
    }
    for (uint32_t i = 0; i < len; i++) {
      outBuffer[i] = buffer[position + i];
    }
    position += len;
  }

  void WriteBytes(uint8_t *buffer, uint32_t len) override {
    if (this->buffer.size() <= position + len) {
      this->buffer.resize(position + len);
    }
    for (uint32_t i = 0; i < len; i++) {
      this->buffer[position + i] = buffer[i];
    }
    position += len;
  }

  void Flush() override {}

  void Close() override {}

  void Drain(std::vector<uint8_t> &buffer) {
    this->buffer.swap(buffer);
    std::vector<uint8_t>().swap(this->buffer);
  }

private:
  std::vector<uint8_t> buffer;
  uint64_t position = 0;
};

#pragma region StdDefinitions.h

enum LicenseType {
  Unused = 0x0000,
  Unrestricted = 0xFFFF,
  ConsoleProfileLicense = 0x0009,
  WindowsProfileLicense = 0x0003,
  ConsoleLicense = 0xF000,
  MediaFlags = 0xE000,
  KeyVaultPrivileges = 0xD000,
  HyperVisorFlags = 0xC000,
  UserPrivileges = 0xB000
};

struct LicenseEntry {
  LicenseType type;
  uint64_t data;
  uint32_t bits;
  uint32_t flags;
};

struct StfsVolumeDescriptor {
  uint8_t size;
  uint8_t reserved;
  uint8_t blockSeperation;
  uint16_t fileTableBlockCount;
  INT24 fileTableBlockNum;
  uint8_t topHashTableHash[0x14];
  uint32_t allocatedBlockCount;
  uint32_t unallocatedBlockCount;
};

struct SvodVolumeDescriptor {
  uint8_t size;
  uint8_t blockCacheElementCount;
  uint8_t workerThreadProcessor;
  uint8_t workerThreadPriority;
  uint8_t rootHash[0x14];
  uint8_t flags;
  INT24 dataBlockCount;
  INT24 dataBlockOffset;
  uint8_t reserved[5];
};

struct Certificate {
  uint16_t publicKeyCertificateSize;
  uint8_t ownerConsoleID[5];
  std::string ownerConsolePartNumber;
  ConsoleType ownerConsoleType;
  ConsoleTypeFlags consoleTypeFlags;
  std::string dateGeneration;
  uint32_t publicExponent;
  uint8_t publicModulus[0x80];
  uint8_t certificateSignature[0x100];
  uint8_t signature[0x80];
};

class StfsHelpers {
  StfsHelpers() = delete;

public:
  static void ReadStfsVolumeDescriptorEx(StfsVolumeDescriptor *descriptor, BaseIO *io, uint32_t address) {
    // seek to the volume descriptor
    io->SetPosition(address);

    // read the descriptor
    descriptor->size = io->ReadByte();
    descriptor->reserved = io->ReadByte();
    descriptor->blockSeperation = io->ReadByte();

    io->SetEndian(LittleEndian);

    descriptor->fileTableBlockCount = io->ReadWord();
    descriptor->fileTableBlockNum = io->ReadInt24();
    io->ReadBytes(descriptor->topHashTableHash, 0x14);

    io->SetEndian(BigEndian);

    descriptor->allocatedBlockCount = io->ReadDword();
    descriptor->unallocatedBlockCount = io->ReadDword();
  }

  static void ReadSvodVolumeDescriptorEx(SvodVolumeDescriptor *descriptor, BaseIO *io) {
    // seek to the volume descriptor
    io->SetPosition(0x379);

    descriptor->size = io->ReadByte();
    descriptor->blockCacheElementCount = io->ReadByte();
    descriptor->workerThreadProcessor = io->ReadByte();
    descriptor->workerThreadPriority = io->ReadByte();

    io->ReadBytes(descriptor->rootHash, 0x14);

    descriptor->flags = io->ReadByte();
    descriptor->dataBlockCount = io->ReadInt24(LittleEndian);
    descriptor->dataBlockOffset = io->ReadInt24(LittleEndian);

    io->ReadBytes(descriptor->reserved, 0x05);
  }

  static void ReadCertificateEx(Certificate *cert, BaseIO *io, uint32_t address) {
    using namespace std;

    // seek to the address of the certificate
    io->SetPosition(address);

    cert->publicKeyCertificateSize = io->ReadWord();
    io->ReadBytes(cert->ownerConsoleID, 5);

    char tempPartNum[0x15];
    tempPartNum[0x14] = 0;
    io->ReadBytes((uint8_t *)tempPartNum, 0x11);
    cert->ownerConsolePartNumber = string(tempPartNum);

    uint32_t temp = io->ReadDword();
    cert->ownerConsoleType = (ConsoleType)(temp & 3);
    cert->consoleTypeFlags = (ConsoleTypeFlags)(temp & 0xFFFFFFFC);
    if (cert->ownerConsoleType != DevKit && cert->ownerConsoleType != Retail)
      throw string("STFS: Invalid console type.\n");

    char tempGenDate[9] = {0};
    io->ReadBytes((uint8_t *)tempGenDate, 8);
    cert->dateGeneration = string(tempGenDate);

    cert->publicExponent = io->ReadDword();
    io->ReadBytes(cert->publicModulus, 0x80);
    io->ReadBytes(cert->certificateSignature, 0x100);
    io->ReadBytes(cert->signature, 0x80);
  }
};
#pragma endregion

#pragma region AvatarAssetDefinitions

enum AssetSubcategory {
  CarryableCarryable = 0x44c,
  CarryableFirst = 0x44c,
  CarryableLast = 0x44c,
  CostumeCasualSuit = 0x68,
  CostumeCostume = 0x69,
  CostumeFirst = 100,
  CostumeFormalSuit = 0x67,
  CostumeLast = 0x6a,
  CostumeLongDress = 0x65,
  CostumeShortDress = 100,
  EarringsDanglers = 0x387,
  EarringsFirst = 900,
  EarringsLargehoops = 0x38b,
  EarringsLast = 0x38b,
  EarringsSingleDangler = 0x386,
  EarringsSingleLargeHoop = 0x38a,
  EarringsSingleSmallHoop = 0x388,
  EarringsSingleStud = 900,
  EarringsSmallHoops = 0x389,
  EarringsStuds = 0x385,
  GlassesCostume = 0x2be,
  GlassesFirst = 700,
  GlassesGlasses = 700,
  GlassesLast = 0x2be,
  GlassesSunglasses = 0x2bd,
  GlovesFingerless = 600,
  GlovesFirst = 600,
  GlovesFullFingered = 0x259,
  GlovesLast = 0x259,
  HatBaseballCap = 0x1f6,
  HatBeanie = 500,
  HatBearskin = 0x1fc,
  HatBrimmed = 0x1f8,
  HatCostume = 0x1fb,
  HatFez = 0x1f9,
  HatFirst = 500,
  HatFlatCap = 0x1f5,
  HatHeadwrap = 0x1fa,
  HatHelmet = 0x1fd,
  HatLast = 0x1fd,
  HatPeakCap = 0x1f7,
  RingFirst = 0x3e8,
  RingLast = 0x3ea,
  RingLeft = 0x3e9,
  RingRight = 0x3e8,
  ShirtCoat = 210,
  ShirtFirst = 200,
  ShirtHoodie = 0xd0,
  ShirtJacket = 0xd1,
  ShirtLast = 210,
  ShirtLongSleeveShirt = 0xce,
  ShirtLongSleeveTee = 0xcc,
  ShirtPolo = 0xcb,
  ShirtShortSleeveShirt = 0xcd,
  ShirtSportsTee = 200,
  ShirtSweater = 0xcf,
  ShirtTee = 0xc9,
  ShirtVest = 0xca,
  ShoesCostume = 0x197,
  ShoesFirst = 400,
  ShoesFormal = 0x193,
  ShoesHeels = 0x191,
  ShoesHighBoots = 0x196,
  ShoesLast = 0x197,
  ShoesPumps = 0x192,
  ShoesSandals = 400,
  ShoesShortBoots = 0x195,
  ShoesTrainers = 0x194,
  TrousersCargo = 0x131,
  TrousersFirst = 300,
  TrousersHotpants = 300,
  TrousersJeans = 0x132,
  TrousersKilt = 0x134,
  TrousersLast = 0x135,
  TrousersLeggings = 0x12f,
  TrousersLongShorts = 0x12e,
  TrousersLongSkirt = 0x135,
  TrousersShorts = 0x12d,
  TrousersShortSkirt = 0x133,
  TrousersTrousers = 0x130,
  WristwearBands = 0x322,
  WristwearBracelet = 800,
  WristwearFirst = 800,
  WristwearLast = 0x323,
  WristwearSweatbands = 0x323,
  WristwearWatch = 0x321
};

enum SkeletonVersion {
  Nxe = 1,
  Natal,
  NxeAndNatal
};

#pragma endregion

#pragma region XdbfHelpers
class XdbfHelpers {
  XdbfHelpers() = delete;

public:
  static time_t FILETIMEtoTimeT(WINFILETIME time) {
    int64_t i64 = (((int64_t)(time.dwHighDateTime)) << 32) + time.dwLowDateTime;
    return (time_t)((i64 - 116444736000000000) / 10000000);
  }
};
#pragma endregion

#pragma region XContentHeader
enum OnlineContentResumeState {
  FileHeadersNotReady = 0x46494C48,
  NewFolder = 0x666F6C64,
  NewFolderResum_Attempt1 = 0x666F6C31,
  NewFolderResumeAttempt2 = 0x666F6C32,
  NewFolderResumeAttemptUnknown = 0x666F6C3F,
  NewFolderResumeAttemptSpecific = 0x666F6C40
};

enum FileSystem {
  FileSystemSTFS = 0,
  FileSystemSVOD,
  FileSystemFATX,
  FileSystemFriendlyFATX,
  FileSystemISO
};

class XContentHeader {
public:
  // Description: read in all of the metadata for the package
  explicit XContentHeader(BaseIO *io) : installerType((InstallerType)0) {
    // set the io
    this->io = io;

    fileSize = io->Length();

    readMetadata();
  }

  ~XContentHeader() {
    delete[] thumbnailImage;
    delete[] titleThumbnailImage;
  }

  uint32_t fileSize;

  Magic magic;

  // only console signed
  Certificate certificate;

  // only strong signed
  uint8_t packageSignature[0x100];

  LicenseEntry licenseData[0x10];
  uint8_t headerHash[0x14];
  uint32_t headerSize;
  ContentType contentType;
  uint32_t metaDataVersion;
  uint64_t contentSize;
  uint32_t mediaID;
  uint32_t version;
  uint32_t baseVersion;
  uint32_t titleID;
  uint8_t platform;
  uint8_t executableType;
  uint8_t discNumber;
  uint8_t discInSet;
  uint32_t savegameID;
  uint8_t consoleID[5];
  uint8_t profileID[8];

  StfsVolumeDescriptor stfsVolumeDescriptor;
  SvodVolumeDescriptor svodVolumeDescriptor;
  FileSystem fileSystem;

  // only in PEC, and im not sure exactly what this byte is but it needs to always be set to 1
  bool enabled;

  // metadata v1
  uint32_t dataFileCount;
  uint64_t dataFileCombinedSize;
  uint8_t deviceID[0x14];
  std::wstring displayName;
  std::wstring displayDescription;
  std::wstring publisherName;
  std::wstring titleName;
  uint8_t transferFlags;
  uint32_t thumbnailImageSize;
  uint32_t titleThumbnailImageSize;

  // credit to Eaton for all the extra metadata stuff
  InstallerType installerType;

  // Avatar Asset
  AssetSubcategory subCategory;
  uint32_t colorizable;
  uint8_t guid[0x10];
  SkeletonVersion skeletonVersion;

  // media
  uint8_t seriesID[0x10];
  uint8_t seasonID[0x10];
  uint16_t seasonNumber;
  uint16_t episodeNumber;

  // installer progress cache data
  OnlineContentResumeState resumeState;
  uint32_t currentFileIndex;
  uint64_t currentFileOffset;
  uint64_t bytesProcessed;
  time_t lastModified;
  uint8_t cabResumeData[5584];

  // installer update data
  Version installerBaseVersion;
  Version installerVersion;

  uint8_t *thumbnailImage;
  uint8_t *titleThumbnailImage;

private:
  BaseIO *io;

  void readMetadata() {
    using namespace std;
    // store errors thrown
    stringstream except;

    io->SetPosition(0);

    magic = (Magic)io->ReadDword();

    if (magic == CON)
      StfsHelpers::ReadCertificateEx(&certificate, io, 4);
    else if (magic == LIVE || magic == PIRS)
      io->ReadBytes(packageSignature, 0x100);
    else {
      except << "XContentHeader: Content signature type 0x" << hex << (uint32_t)magic << " is invalid.\n";
      throw except.str();
    }

    io->SetPosition(0x22C);

    // read licensing data
    for (int i = 0; i < 0x10; i++) {
      uint64_t tempYo = io->ReadUInt64();
      licenseData[i].type = (LicenseType)(tempYo >> 48);
      licenseData[i].data = (tempYo & 0xFFFFFFFFFFFF);
      licenseData[i].bits = io->ReadDword();
      licenseData[i].flags = io->ReadDword();

      switch (licenseData[i].type) {
      case 0:
      case 0xFFFF:
      case 9:
      case 3:
      case 0xF000:
      case 0xE000:
      case 0xD000:
      case 0xC000:
      case 0xB000:
        break;
      default:
        except << "XContentHeader: Invalid license type at index " << i << ".\n";
        throw except.str();
      }
    }

    // header hash / content id
    io->ReadBytes(headerHash, 0x14);

    headerSize = io->ReadDword();

    // to do: make sure it's a valid type
    contentType = (ContentType)io->ReadDword();

    // read metadata information
    metaDataVersion = io->ReadDword();
    contentSize = io->ReadUInt64();
    mediaID = io->ReadDword();
    version = io->ReadDword();
    baseVersion = io->ReadDword();
    titleID = io->ReadDword();
    platform = io->ReadByte();
    executableType = io->ReadByte();
    discNumber = io->ReadByte();
    discInSet = io->ReadByte();
    savegameID = io->ReadDword();
    io->ReadBytes(consoleID, 5);
    io->ReadBytes(profileID, 8);

    // read the file system type
    io->SetPosition(0x3A9);
    fileSystem = (FileSystem)io->ReadDword();
    if (fileSystem > 1)
      throw string("XContentHeader: Invalid file system. Only STFS and SVOD are supported.\n");

    // read volume descriptor
    if (fileSystem == FileSystemSTFS)
      StfsHelpers::ReadStfsVolumeDescriptorEx(&stfsVolumeDescriptor, io, 0x379);
    else if (fileSystem == FileSystemSVOD)
      StfsHelpers::ReadSvodVolumeDescriptorEx(&svodVolumeDescriptor, io);

    dataFileCount = io->ReadDword();
    dataFileCombinedSize = io->ReadUInt64();

    // read the avatar metadata if needed
    if (contentType == AvatarItem) {
      io->SetPosition(0x3D9);
      io->SwapEndian();

      subCategory = (AssetSubcategory)io->ReadDword();
      colorizable = io->ReadDword();

      io->SwapEndian();

      io->ReadBytes(guid, 0x10);
      skeletonVersion = (SkeletonVersion)io->ReadByte();
    } else if (contentType == Video) // there may be other content types with this metadata
    {
      io->SetPosition(0x3D9);

      io->ReadBytes(seriesID, 0x10);
      io->ReadBytes(seasonID, 0x10);

      seasonNumber = io->ReadWord();
      episodeNumber = io->ReadWord();
    }

    // skip padding
    io->SetPosition(0x3FD);

    io->ReadBytes(deviceID, 0x14);
    displayName = io->ReadWString();
    io->SetPosition(0xD11);
    displayDescription = io->ReadWString();
    io->SetPosition(0x1611);
    publisherName = io->ReadWString(0x80);
    io->SetPosition(0x1691);
    titleName = io->ReadWString(0x80);
    io->SetPosition(0x1711);
    transferFlags = io->ReadByte();

    // read image sizes
    thumbnailImageSize = io->ReadDword();
    titleThumbnailImageSize = io->ReadDword();

    thumbnailImage = new uint8_t[thumbnailImageSize];
    titleThumbnailImage = new uint8_t[titleThumbnailImageSize];

    // read images
    io->ReadBytes(thumbnailImage, thumbnailImageSize);
    io->SetPosition(0x571A);

    if (thumbnailImageSize == 0 || thumbnailImage[0] == 0)
      thumbnailImage = NULL;

    io->ReadBytes(titleThumbnailImage, titleThumbnailImageSize);
    io->SetPosition(0x971A);

    if (titleThumbnailImageSize == 0 || titleThumbnailImage[0] == 0)
      thumbnailImage = NULL;

    if (((headerSize + 0xFFF) & 0xFFFFF000) - 0x971A < 0x15F4)
      return;

    installerType = (InstallerType)io->ReadDword();
    switch (installerType) {
    case SystemUpdate:
    case TitleUpdate: {
      uint32_t tempbv = io->ReadDword();
      installerBaseVersion.major = tempbv >> 28;
      installerBaseVersion.minor = (tempbv >> 24) & 0xF;
      installerBaseVersion.build = (tempbv >> 8) & 0xFFFF;
      installerBaseVersion.revision = tempbv & 0xFF;

      uint32_t tempv = io->ReadDword();
      installerVersion.major = tempv >> 28;
      installerVersion.minor = (tempv >> 24) & 0xF;
      installerVersion.build = (tempv >> 8) & 0xFFFF;
      installerVersion.revision = tempv & 0xFF;

      break;
    }
    case SystemUpdateProgressCache:
    case TitleUpdateProgressCache:
    case TitleContentProgressCache: {
      resumeState = (OnlineContentResumeState)io->ReadDword();
      currentFileIndex = io->ReadDword();
      currentFileOffset = io->ReadUInt64();
      bytesProcessed = io->ReadUInt64();

      WINFILETIME time;
      time.dwHighDateTime = io->ReadDword();
      time.dwLowDateTime = io->ReadDword();
      lastModified = XdbfHelpers::FILETIMEtoTimeT(time);

      io->ReadBytes(cabResumeData, 0x15D0);
      break;
    }
    case None:
      break;
    default:
      throw string("XContentHeader: Invalid Installer Type value.");
    }

#ifdef DEBUG
    if (metaDataVersion != 2)
      throw string("XContentHeader: Metadata version is not 2.\n");
#endif
  }
};

#pragma endregion

#pragma region StfsPackage.h

struct StfsFileEntry {
  uint32_t entryIndex;
  std::string name;
  uint8_t nameLen;
  uint8_t flags;
  INT24 blocksForFile;
  INT24 startingBlockNum;
  uint16_t pathIndicator;
  uint32_t fileSize;
  uint32_t createdTimeStamp;
  uint32_t accessTimeStamp;
  uint32_t fileEntryAddress;
  std::vector<INT24> blockChain;
};

struct StfsFileListing {
  std::vector<StfsFileEntry> fileEntries;
  std::vector<StfsFileListing> folderEntries;
  StfsFileEntry folder;
};

#pragma pack(push, 1)
struct HashEntry {
  uint8_t blockHash[0x14];
  uint8_t status;
  uint32_t nextBlock;
};
#pragma pack(pop)

struct HashTable {
  Level level;
  uint32_t trueBlockNumber;
  uint32_t entryCount;
  HashEntry entries[0xAA];
  uint32_t addressInFile;
};

class StfsPackage {
public:
  // Description: initialize a stfs package
  explicit StfsPackage(std::string packagePath) : ioPassedIn(false) {
    metaData = NULL;

    io = new FileIO(packagePath, false);
    try {
      Init();
    } catch (std::string &) {
      Cleanup();
      throw;
    }
  }

  // Description: get the file listing of the package, forceUpdate reads from the package regardless
  StfsFileListing GetFileListing(bool forceUpdate = false) {
    // update the file listing from file if requested
    if (forceUpdate)
      ReadFileListing();

    return fileListing;
  }

  // Description: extract a file (by FileEntry) to a designated file path
  void ExtractFile(StfsFileEntry *entry, std::string outPath, void (*extractProgress)(void *, uint32_t, uint32_t) = NULL, void *arg = NULL) {
    // create/truncate our out file
    FileIO outFile(outPath, true);
    Extract(entry, outFile, extractProgress);
  }

  // Description: extract a file (by FileEntry) to a designated file path
  void Extract(StfsFileEntry *entry, BaseIO &out, void (*extractProgress)(void *, uint32_t, uint32_t) = NULL, void *arg = NULL) {
    if (entry->nameLen == 0) {
      except.str(std::string());
      except << "STFS: File '" << entry->name.c_str() << "' doesn't exist in the package.\n";
      throw except.str();
    }

    // get the file size that we are extracting
    uint32_t fileSize = entry->fileSize;

    // make a special case for files of size 0
    if (fileSize == 0) {
      out.Close();

      // update progress if needed
      if (extractProgress != NULL)
        extractProgress(arg, 1, 1);

      return;
    }

    // check if all the blocks are consecutive
    if (entry->flags & 1) {
      // allocate 0xAA blocks of memory, for maximum efficiency, yo
      uint8_t *buffer = new uint8_t[0xAA000];

      // seek to the begining of the file
      uint32_t startAddress = BlockToAddress(entry->startingBlockNum);
      io->SetPosition(startAddress);

      // calculate the number of blocks to read before we hit a table
      uint32_t blockCount = (ComputeLevel0BackingHashBlockNumber(entry->startingBlockNum) + blockStep[0]) - ((startAddress - firstHashTableAddress) >> 0xC);

      // pick up the change at the begining, until we hit a hash table
      if ((uint32_t)entry->blocksForFile <= blockCount) {
        io->ReadBytes(buffer, entry->fileSize);
        out.Write(buffer, entry->fileSize);

        // update progress if needed
        if (extractProgress != NULL)
          extractProgress(arg, entry->blocksForFile, entry->blocksForFile);

        out.Close();

        // free the temp buffer
        delete[] buffer;
        return;
      } else {
        io->ReadBytes(buffer, blockCount << 0xC);
        out.Write(buffer, blockCount << 0xC);

        // update progress if needed
        if (extractProgress != NULL)
          extractProgress(arg, blockCount, entry->blocksForFile);
      }

      // extract the blocks inbetween the tables
      uint32_t tempSize = (entry->fileSize - (blockCount << 0xC));
      while (tempSize >= 0xAA000) {
        // skip past the hash table(s)
        uint32_t currentPos = io->GetPosition();
        io->SetPosition(currentPos + GetHashTableSkipSize(currentPos));

        // read in the 0xAA blocks between the tables
        io->ReadBytes(buffer, 0xAA000);

        // Write the bytes to the out file
        out.Write(buffer, 0xAA000);

        tempSize -= 0xAA000;
        blockCount += 0xAA;

        // update progress if needed
        if (extractProgress != NULL)
          extractProgress(arg, blockCount, entry->blocksForFile);
      }

      // pick up the change at the end
      if (tempSize != 0) {
        // skip past the hash table(s)
        uint32_t currentPos = io->GetPosition();
        io->SetPosition(currentPos + GetHashTableSkipSize(currentPos));

        // read in the extra crap
        io->ReadBytes(buffer, tempSize);

        // Write it to the out file
        out.Write(buffer, tempSize);

        // update progress if needed
        if (extractProgress != NULL)
          extractProgress(arg, entry->blocksForFile, entry->blocksForFile);
      }

      // free the temp buffer
      delete[] buffer;
    } else {
      // generate the block chain which we have to extract
      uint32_t fullReadCounts = fileSize / 0x1000;

      fileSize -= (fullReadCounts * 0x1000);

      uint32_t block = entry->startingBlockNum;

      // allocate data for the blocks
      uint8_t data[0x1000];

      // read all the full blocks the file allocates
      for (uint32_t i = 0; i < fullReadCounts; i++) {
        ExtractBlock(block, data);
        out.Write(data, 0x1000);

        block = GetBlockHashEntry(block).nextBlock;

        // call the extract progress function if needed
        if (extractProgress != NULL)
          extractProgress(arg, i + 1, entry->blocksForFile);
      }

      // read the remaining data
      if (fileSize != 0) {
        ExtractBlock(block, data, fileSize);
        out.Write(data, fileSize);

        // call the extract progress function if needed
        if (extractProgress != NULL)
          extractProgress(arg, entry->blocksForFile, entry->blocksForFile);
      }
    }

    // cleanup
    out.Close();
  }

  // Description: convert a block into an address in the file
  uint32_t BlockToAddress(uint32_t blockNum) {
    // check for invalid block number
    if (blockNum >= INT24_MAX)
      throw std::string("STFS: Block number must be less than 0xFFFFFF.\n");
    return (ComputeBackingDataBlockNumber(blockNum) << 0x0C) + firstHashTableAddress;
  }

  // Description: get the address of a hash for a data block
  uint32_t GetHashAddressOfBlock(uint32_t blockNum) {
    if (blockNum >= metaData->stfsVolumeDescriptor.allocatedBlockCount)
      throw std::string("STFS: Reference to illegal block number.\n");

    uint32_t hashAddr = (ComputeLevel0BackingHashBlockNumber(blockNum) << 0xC) + firstHashTableAddress;
    hashAddr += (blockNum % 0xAA) * 0x18;

    switch (topLevel) {
    case 0:
      hashAddr += ((metaData->stfsVolumeDescriptor.blockSeperation & 2) << 0xB);
      break;
    case 1:
      hashAddr += ((topTable.entries[blockNum / 0xAA].status & 0x40) << 6);
      break;
    case 2:
      uint32_t level1Off = ((topTable.entries[blockNum / 0x70E4].status & 0x40) << 6);
      uint32_t pos = ((ComputeLevel1BackingHashBlockNumber(blockNum) << 0xC) + firstHashTableAddress + level1Off) + ((blockNum % 0xAA) * 0x18);
      io->SetPosition(pos + 0x14);
      hashAddr += ((io->ReadByte() & 0x40) << 6);
      break;
    }
    return hashAddr;
  }

  ~StfsPackage(void) {
    Cleanup();
  }

  XContentHeader const *GetMetaData() const {
    return metaData;
  }

private:
  XContentHeader *metaData;

  StfsFileListing fileListing;
  StfsFileListing writtenToFile;

  BaseIO *io;
  std::stringstream except;
  bool ioPassedIn;

  Sex packageSex;
  uint32_t blockStep[2];
  uint32_t firstHashTableAddress;
  uint8_t hashOffset;
  Level topLevel;
  HashTable topTable;
  HashTable cached;
  uint32_t tablesPerLevel[3];

  // Description: read the file listing from the file
  void ReadFileListing() {
    fileListing.fileEntries.clear();
    fileListing.folderEntries.clear();

    // setup the entry for the block chain
    StfsFileEntry entry;
    entry.startingBlockNum = metaData->stfsVolumeDescriptor.fileTableBlockNum;
    entry.fileSize = (metaData->stfsVolumeDescriptor.fileTableBlockCount * 0x1000);

    // generate a block chain for the full file listing
    uint32_t block = entry.startingBlockNum;

    StfsFileListing fl;
    uint32_t currentAddr;
    for (uint32_t x = 0; x < metaData->stfsVolumeDescriptor.fileTableBlockCount; x++) {
      currentAddr = BlockToAddress(block);
      io->SetPosition(currentAddr);

      for (uint32_t i = 0; i < 0x40; i++) {
        StfsFileEntry fe;

        // set the current position
        fe.fileEntryAddress = currentAddr + (i * 0x40);

        // calculate the entry index (in the file listing)
        fe.entryIndex = (x * 0x40) + i;

        // read the name, if the length is 0 then break
        fe.name = io->ReadString(0x28);

        // read the name length
        fe.nameLen = io->ReadByte();
        if ((fe.nameLen & 0x3F) == 0) {
          io->SetPosition(currentAddr + ((i + 1) * 0x40));
          continue;
        } else if (fe.name.length() == 0)
          break;

        // check for a mismatch in the total allocated blocks for the file
        fe.blocksForFile = io->ReadInt24(LittleEndian);
        io->SetPosition(3, std::ios_base::cur);

        // read more information
        fe.startingBlockNum = io->ReadInt24(LittleEndian);
        fe.pathIndicator = io->ReadWord();
        fe.fileSize = io->ReadDword();
        fe.createdTimeStamp = io->ReadDword();
        fe.accessTimeStamp = io->ReadDword();

        // get the flags
        fe.flags = fe.nameLen >> 6;

        // bits 6 and 7 are flags, clear them
        fe.nameLen &= 0x3F;

        fl.fileEntries.push_back(fe);
      }

      block = GetBlockHashEntry(block).nextBlock;
    }

    // sort the file listing
    AddToListing(&fl, &fileListing);
    writtenToFile = fileListing;
  }

  // Description: extract a block's data
  void ExtractBlock(uint32_t blockNum, uint8_t *data, uint32_t length = 0x1000) {
    if (blockNum >= metaData->stfsVolumeDescriptor.allocatedBlockCount)
      throw std::string("STFS: Reference to illegal block number.\n");

    // check for an invalid block length
    if (length > 0x1000)
      throw std::string("STFS: length cannot be greater 0x1000.\n");

    // go to the block's position
    io->SetPosition(BlockToAddress(blockNum));

    // read the data, and return
    io->ReadBytes(data, length);
  }

  // Description: convert a block number into a true block number, where the first block is the first hash table
  uint32_t ComputeBackingDataBlockNumber(uint32_t blockNum) {
    uint32_t toReturn = (((blockNum + 0xAA) / 0xAA) << (uint8_t)packageSex) + blockNum;
    if (blockNum < 0xAA)
      return toReturn;
    else if (blockNum < 0x70E4)
      return toReturn + (((blockNum + 0x70E4) / 0x70E4) << (uint8_t)packageSex);
    else
      return (1 << (uint8_t)packageSex) + (toReturn + (((blockNum + 0x70E4) / 0x70E4) << (uint8_t)packageSex));
  }

  // Description: get a block's hash entry
  HashEntry GetBlockHashEntry(uint32_t blockNum) {
    if (blockNum >= metaData->stfsVolumeDescriptor.allocatedBlockCount)
      throw std::string("STFS: Reference to illegal block number.\n");

    // go to the position of the hash address
    io->SetPosition(GetHashAddressOfBlock(blockNum));

    // read the hash entry
    HashEntry he;
    io->ReadBytes(he.blockHash, 0x14);
    he.status = io->ReadByte();
    he.nextBlock = io->ReadInt24();

    return he;
  }

  // Description: get the true block number for the hash table that hashes the block at the level passed in
  uint32_t ComputeLevelNBackingHashBlockNumber(uint32_t blockNum, Level level) {
    switch (level) {
    case Zero:
      return ComputeLevel0BackingHashBlockNumber(blockNum);

    case One:
      return ComputeLevel1BackingHashBlockNumber(blockNum);

    case Two:
      return ComputeLevel2BackingHashBlockNumber(blockNum);

    default:
      throw std::string("STFS: Invalid level.\n");
    }
  }

  // Description: get the true block number for the hash table that hashes the block at level 0
  uint32_t ComputeLevel0BackingHashBlockNumber(uint32_t blockNum) {
    if (blockNum < 0xAA)
      return 0;

    uint32_t num = (blockNum / 0xAA) * blockStep[0];
    num += ((blockNum / 0x70E4) + 1) << ((uint8_t)packageSex);

    if (blockNum / 0x70E4 == 0)
      return num;

    return num + (1 << (uint8_t)packageSex);
  }

  // Description: get the true block number for the hash table that hashes the block at level 1 (female)
  uint32_t ComputeLevel1BackingHashBlockNumber(uint32_t blockNum) {
    if (blockNum < 0x70E4)
      return blockStep[0];
    return (1 << (uint8_t)packageSex) + (blockNum / 0x70E4) * blockStep[1];
  }

  // Description: get the true block number for the hash table that hashes the block at level 2
  uint32_t ComputeLevel2BackingHashBlockNumber(uint32_t blockNum) {
    return blockStep[1];
  }

  // Description: add the file entry to the file listing
  void AddToListing(StfsFileListing *fullListing, StfsFileListing *out) {
    for (uint32_t i = 0; i < fullListing->fileEntries.size(); i++) {
      // check if the file is a directory
      bool isDirectory = (fullListing->fileEntries.at(i).flags & 2);

      // make sure the file belongs to the current folder
      if (fullListing->fileEntries.at(i).pathIndicator == out->folder.entryIndex) {
        // add it if it's a file
        if (!isDirectory)
          out->fileEntries.push_back(fullListing->fileEntries.at(i));
        // if it's a directory and not the current directory, then add it
        else if (isDirectory && fullListing->fileEntries.at(i).entryIndex != out->folder.entryIndex) {
          StfsFileListing fl;
          fl.folder = fullListing->fileEntries.at(i);
          out->folderEntries.push_back(fl);
        }
      }
    }

    // for every folder added, add the files to them
    for (uint32_t i = 0; i < out->folderEntries.size(); i++)
      AddToListing(fullListing, &out->folderEntries.at(i));
  }

  // Description: calculate the level of the topmost hash table
  Level CalcualateTopLevel() {
    if (metaData->stfsVolumeDescriptor.allocatedBlockCount <= 0xAA)
      return Zero;
    else if (metaData->stfsVolumeDescriptor.allocatedBlockCount <= 0x70E4)
      return One;
    else if (metaData->stfsVolumeDescriptor.allocatedBlockCount <= 0x4AF768)
      return Two;
    else
      throw std::string("STFS: Invalid number of allocated blocks.\n");
  }

  // Description: get the number of bytes to skip over the hash table
  uint32_t GetHashTableSkipSize(uint32_t tableAddress) {
    // convert the address to a true block number
    uint32_t trueBlockNumber = (tableAddress - firstHashTableAddress) >> 0xC;

    // check if it's the first hash table
    if (trueBlockNumber == 0)
      return (0x1000 << packageSex);

    // check if it's the level 2 table, or above
    if (trueBlockNumber == blockStep[1])
      return (0x3000 << packageSex);
    else if (trueBlockNumber > blockStep[1])
      trueBlockNumber -= (blockStep[1] + (1 << packageSex));

    // check if it's at a level 1 table
    if (trueBlockNumber == blockStep[0] || trueBlockNumber % blockStep[1] == 0)
      return (0x2000 << packageSex);

    // otherwise, assume it's at a level 0 table
    return (0x1000 << packageSex);
  }

  // Description: parse the file
  void Parse() {
    using namespace std;
    metaData = new XContentHeader(io);

    // make sure the file system is STFS
    if (metaData->fileSystem != FileSystemSTFS)
      throw string("STFS: Invalid file system header.\n");

    packageSex = (Sex)((~metaData->stfsVolumeDescriptor.blockSeperation) & 1);

    if (packageSex == StfsFemale) {
      blockStep[0] = 0xAB;
      blockStep[1] = 0x718F;
    } else {
      blockStep[0] = 0xAC;
      blockStep[1] = 0x723A;
    }

    // address of the first hash table in the package, comes right after the header
    firstHashTableAddress = (metaData->headerSize + 0x0FFF) & 0xFFFFF000;

    // calculate the number of tables per level
    tablesPerLevel[0] = (metaData->stfsVolumeDescriptor.allocatedBlockCount / 0xAA) + ((metaData->stfsVolumeDescriptor.allocatedBlockCount % 0xAA != 0) ? 1 : 0);
    tablesPerLevel[1] = (tablesPerLevel[0] / 0xAA) + ((tablesPerLevel[0] % 0xAA != 0 && metaData->stfsVolumeDescriptor.allocatedBlockCount > 0xAA) ? 1 : 0);
    tablesPerLevel[2] = (tablesPerLevel[1] / 0xAA) + ((tablesPerLevel[1] % 0xAA != 0 && metaData->stfsVolumeDescriptor.allocatedBlockCount > 0x70E4) ? 1 : 0);

    // calculate the level of the top table
    topLevel = CalcualateTopLevel();

    // read in the top hash table
    topTable.trueBlockNumber = ComputeLevelNBackingHashBlockNumber(0, topLevel);
    topTable.level = topLevel;

    uint32_t baseAddress = (topTable.trueBlockNumber << 0xC) + firstHashTableAddress;
    topTable.addressInFile = baseAddress + ((metaData->stfsVolumeDescriptor.blockSeperation & 2) << 0xB);
    io->SetPosition(topTable.addressInFile);

    uint32_t dataBlocksPerHashTreeLevel[3] = {1, 0xAA, 0x70E4};

    // load the information
    topTable.entryCount = metaData->stfsVolumeDescriptor.allocatedBlockCount / dataBlocksPerHashTreeLevel[topLevel];
    if (metaData->stfsVolumeDescriptor.allocatedBlockCount > 0x70E4 && (metaData->stfsVolumeDescriptor.allocatedBlockCount % 0x70E4 != 0))
      topTable.entryCount++;
    else if (metaData->stfsVolumeDescriptor.allocatedBlockCount > 0xAA && (metaData->stfsVolumeDescriptor.allocatedBlockCount % 0xAA != 0))
      topTable.entryCount++;

    for (uint32_t i = 0; i < topTable.entryCount; i++) {
      io->ReadBytes(topTable.entries[i].blockHash, 0x14);
      topTable.entries[i].status = io->ReadByte();
      topTable.entries[i].nextBlock = io->ReadInt24();
    }

    // set default values for the root of the file listing
    StfsFileEntry fe;
    fe.pathIndicator = 0xFFFF;
    fe.name = "Root";
    fe.entryIndex = 0xFFFF;
    fileListing.folder = fe;

    ReadFileListing();
  }

  // Description: initializes the object
  void Init() {
    Parse();
  }

  // Description: close the io/cleanup resources
  void Cleanup() {
    io->Close();

    if (!ioPassedIn)
      delete io;
    if (metaData)
      delete metaData;
  }
};
#pragma endregion

} // namespace je2be::box360

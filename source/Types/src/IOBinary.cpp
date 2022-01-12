/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2021, ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <Types/include/IOBinary.h>
#include <array>
#include <fstream>
#include <iostream>

namespace haptics::types {

auto IOBinary::loadFile(const std::string &filePath) -> bool {
  std::ifstream file(filePath, std::ios::binary | std::ifstream::in);
  if (!file) {
    std::cout << filePath << ": Cannot open file!" << std::endl;
    file.close();
    return false;
  }

  file.seekg(0, std::ios::end);
  unsigned int length = static_cast<unsigned int>(file.tellg());
  file.seekg(0, std::ios::beg);

  std::cout << "Open: " << length << std::endl;
  if (length == 0) { // avoid undefined behavior
    file.close();
    return false;
  }

  Haptics haptic;
  if (!IOBinary::readFileHeader(haptic, file)) {
    file.close();
    return false;
  }

  file.close();
  return true;
}

auto IOBinary::writeFile(types::Haptics &haptic, const std::string &filePath) -> bool {
  std::ofstream file(filePath, std::ios::out | std::ios::binary);
  if (!file) {
    std::cout << filePath << ": Cannot open file!" << std::endl;
    return false;
  }

  bool res = IOBinary::writeFileHeader(haptic, file);
  file.close();
  return res;
}

auto IOBinary::readFileHeader(types::Haptics &haptic, std::ifstream &file) -> bool {
  std::string version = IOBinary::readString(file);
  std::string date = IOBinary::readString(file);
  std::string description = IOBinary::readString(file);
  haptic.setVersion(version);
  haptic.setDate(date);
  haptic.setDescription(description);

  // Get avatars
  if (!IOBinary::readAvatars(haptic, file)) {
    return false;
  }

  std::string shape = IOBinary::readString(file);

  // Get the perception_count
  unsigned short perceptionCount = 0;
  std::array<char, 2> perceptionCountBytes{};
  file.read(perceptionCountBytes.data(), 2);
  std::reverse(perceptionCountBytes.begin(), perceptionCountBytes.end());
  memcpy(&perceptionCount, &perceptionCountBytes, sizeof(perceptionCount));

  return true;
}

auto IOBinary::writeFileHeader(types::Haptics &haptic, std::ofstream &file) -> bool {
  const std::string version = haptic.getVersion();
  const std::string date = haptic.getDate();
  const std::string description = haptic.getDescription();
  const std::string shape = "Custom";
  unsigned short perceptionCount = 0;

  IOBinary::writeString(version, file);
  IOBinary::writeString(date, file);
  IOBinary::writeString(description, file);

  if (!IOBinary::writeAvatars(haptic, file)) {
    return false;
  }

  IOBinary::writeString(shape, file);

  // Write perceptions
  std::array<char, 2> perceptionCountBytes{};
  memcpy(&perceptionCountBytes, &perceptionCount, sizeof(perceptionCount));
  std::reverse(perceptionCountBytes.begin(), perceptionCountBytes.end());
  file.write(perceptionCountBytes.data(), 2);
  for (unsigned short i = 0; i <= perceptionCount; i++) {
    // Write each perception
  }

  return true;
}

auto IOBinary::readAvatars(types::Haptics &haptic, std::ifstream &file) -> bool {
  unsigned short avatarCount = 0;
  std::array<char, 2> avatarCountBytes{};
  file.read(avatarCountBytes.data(), 2);
  std::reverse(avatarCountBytes.begin(), avatarCountBytes.end());
  memcpy(&avatarCount, &avatarCountBytes, sizeof(avatarCount));

  Avatar myAvatar;
  for (unsigned short i = 0; i < avatarCount; i++) {
    short avatarId = 0;
    std::array<char, 2> avatarIdBytes{};
    file.read(avatarIdBytes.data(), 2);
    std::reverse(avatarIdBytes.begin(), avatarIdBytes.end());
    memcpy(&avatarId, &avatarIdBytes, sizeof(avatarId));

    int avatarLod = 0;
    std::array<char, 4> avatarLodBytes{};
    file.read(avatarLodBytes.data(), 4);
    std::reverse(avatarLodBytes.begin(), avatarLodBytes.end());
    memcpy(&avatarLod, &avatarLodBytes, sizeof(avatarLod));

    unsigned short avatarType = 0;
    std::array<char, 2> avatarTypeBytes{};
    file.read(avatarTypeBytes.data(), 2);
    std::reverse(avatarTypeBytes.begin(), avatarTypeBytes.end());
    memcpy(&avatarType, &avatarTypeBytes, sizeof(avatarType));

    std::string avatarURI;
    if (myAvatar.getType() == AvatarType::Custom) {
      // TODO : do something with this value
      avatarURI = IOBinary::readString(file);
    }

    myAvatar = Avatar(avatarId, avatarLod, static_cast<AvatarType>(avatarType));
    haptic.addAvatar(myAvatar);
  }

  return true;
}

auto IOBinary::writeAvatars(types::Haptics &haptic, std::ofstream &file) -> bool {
  auto avatarCount = static_cast<unsigned short>(haptic.getAvatarsSize());

  std::array<char, 2> avatarCountBytes{};
  memcpy(&avatarCountBytes, &avatarCount, sizeof(avatarCount));
  std::reverse(avatarCountBytes.begin(), avatarCountBytes.end());
  file.write(avatarCountBytes.data(), 2);

  Avatar myAvatar;
  const std::string avatarURI = "placeholder";
  for (unsigned short i = 0; i < avatarCount; i++) {
    myAvatar = haptic.getAvatarAt(i);

    auto avatarId = static_cast<short>(myAvatar.getId());
    std::array<char, 2> avatarIdBytes{};
    memcpy(&avatarIdBytes, &avatarId, sizeof(avatarId));
    std::reverse(avatarIdBytes.begin(), avatarIdBytes.end());
    file.write(avatarIdBytes.data(), 2);

    int avatarLod = myAvatar.getLod();
    std::array<char, 4> avatarLodBytes{};
    memcpy(&avatarLodBytes, &avatarLod, sizeof(avatarLod));
    std::reverse(avatarLodBytes.begin(), avatarLodBytes.end());
    file.write(avatarLodBytes.data(), 4);

    auto avatarType = static_cast<unsigned short>(myAvatar.getType());
    std::array<char, 2> avatarTypeBytes{};
    memcpy(&avatarTypeBytes, &avatarType, sizeof(avatarType));
    std::reverse(avatarTypeBytes.begin(), avatarTypeBytes.end());
    file.write(avatarTypeBytes.data(), 2);

    if (myAvatar.getType() == AvatarType::Custom) {
      IOBinary::writeString(avatarURI, file); 
    }
  }
  return true;
}

// NOLINTNEXTLINE(misc-unused-parameters)
auto IOBinary::writeBandHeader(types::Haptics &haptic, std::ofstream &file) -> bool {
  return false;
}

// NOLINTNEXTLINE(misc-unused-parameters)
auto IOBinary::writeBandBody(types::Haptics &haptic, std::ofstream &file) -> bool {
  return false;
}

auto IOBinary::writeString(const std::string &text, std::ofstream &file) -> void {
  std::string str = text;
  str.append(1, '\x00');
  file.write(str.c_str(), static_cast<int>(str.size()));
}

auto IOBinary::readString(std::ifstream &file) -> std::string {
  char c = 0;
  std::string str;
  while (file.get(c), c != '\0') {
    str += c;
  }
  return str;
}
} // namespace haptics::types
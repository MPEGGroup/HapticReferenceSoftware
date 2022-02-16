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

#include <Tools/include/OHMData.h>
#include <algorithm>
#include <array>
#include <cstring>
#include <fstream>
#include <iostream>

namespace haptics::tools {
OHMData::OHMData(const std::string &filePath) { loadFile(filePath); }

auto OHMData::loadFile(const std::string &filePath) -> bool {
  version = 1;
  std::ifstream file(filePath, std::ios::binary | std::ifstream::in);
  if (!file) {
    std::cerr << filePath << ": Cannot open file!" << std::endl;
    file.close();
    return false;
  }

  file.seekg(0, std::ios::end);
  unsigned int length = static_cast<unsigned int>(file.tellg());
  file.seekg(0, std::ios::beg);

  if (length == 0) { // avoid undefined behavior
    std::cerr << "Opening empty file" << std::endl;
    file.close();
    return false;
  }

  // Get the header and check its value
  std::array<char, 4> headerBytes{};
  file.read(headerBytes.data(), 4);
  header = std::string(headerBytes.data(), 4);
  if (header != "OHM ") {
    std::cerr << "Incorrect header: " << header << std::endl;
    file.close();
    return false;
  }

  // Get the OHM version
  std::array<char, 2> versionBytes{};
  file.read(versionBytes.data(), 2);
  std::reverse(versionBytes.begin(), versionBytes.end());
  memcpy(&version, &versionBytes, sizeof(version));

  // Get the number of elements
  std::array<char, 2> numElementsBytes{};
  file.read(numElementsBytes.data(), 2);
  std::reverse(numElementsBytes.begin(), numElementsBytes.end());
  memcpy(&numElements, &numElementsBytes, sizeof(numElements));

  // Get the file description
  std::array<char, descriptionByteSize> descriptionBytes{};
  file.read(descriptionBytes.data(), descriptionByteSize);
  description = std::string(descriptionBytes.data(), descriptionByteSize);
  description.erase(description.find_last_not_of('\x00') + 1);

  for (int i = 0; i < numElements; i++) {
    HapticElementMetadata element;
    std::array<char, fileNameByteSize> hapticObjectFileNameBytes{};
    file.read(hapticObjectFileNameBytes.data(), fileNameByteSize);
    element.elementFilename = std::string(hapticObjectFileNameBytes.data(), fileNameByteSize);
    element.elementFilename.erase(element.elementFilename.find_last_not_of('\x00') + 1);

    std::array<char, descriptionByteSize> elementDescriptionBytes{};
    file.read(elementDescriptionBytes.data(), descriptionByteSize);
    element.elementDescription = std::string(elementDescriptionBytes.data(), descriptionByteSize);
    element.elementDescription.erase(element.elementDescription.find_last_not_of('\x00') + 1);

    std::array<char, 2> numHapticChannelsBytes{};
    file.read(numHapticChannelsBytes.data(), 2);
    std::reverse(numHapticChannelsBytes.begin(), numHapticChannelsBytes.end());
    memcpy(&element.numHapticChannels, &numHapticChannelsBytes, sizeof(element.numHapticChannels));

    for (int j = 0; j < element.numHapticChannels; j++) {
      HapticChannelMetadata channel;
      std::array<char, descriptionByteSize> channelDescriptionBytes{};
      file.read(channelDescriptionBytes.data(), descriptionByteSize);
      channel.channelDescription = std::string(channelDescriptionBytes.data(), descriptionByteSize);
      channel.channelDescription.erase(channel.channelDescription.find_last_not_of('\x00') + 1);

      std::array<char, 4> gainBytes{};
      file.read(gainBytes.data(), 4);
      channel.gain = 0;
      memcpy(&channel.gain, &gainBytes, sizeof(channel.gain));

      std::array<char, 4> bodyPartMaskBytes{};
      file.read(bodyPartMaskBytes.data(), 4);
      channel.bodyPartMask = static_cast<Body>(
          (bodyPartMaskBytes[0] << threeBytesShift) | (bodyPartMaskBytes[1] << twoBytesShift) |
          (bodyPartMaskBytes[2] << oneByteShift) | bodyPartMaskBytes[3]);

      element.channelsMetadata.push_back(channel);
    }
    elementsMetadata.push_back(element);
  }
  file.close();
  return true;
}

auto OHMData::fillString(const std::string &text, const unsigned int numCharacters) -> std::string {
  std::string res = text.length() <= numCharacters ? text : text.substr(0, numCharacters);
  unsigned int numPad = numCharacters - static_cast<unsigned int>(res.length());
  res.append(numPad, '\x00');
  return res;
}

auto OHMData::writeFile(const std::string &filePath) -> bool {
  std::ofstream file(filePath, std::ios::out | std::ios::binary);
  if (!file) {
    std::cerr << filePath << ": Cannot open file!" << std::endl;
    return false;
  }
  // Writing the header
  file.write(header.c_str(), 4);

  // Writing the version
  std::array<char, 2> versionBytes{};
  memcpy(&versionBytes, &version, sizeof(version));
  std::reverse(versionBytes.begin(), versionBytes.end());
  file.write(versionBytes.data(), 2);

  // Writing the number of elements
  std::array<char, 2> numElementsBytes{};
  memcpy(&numElementsBytes, &numElements, sizeof(numElements));
  std::reverse(numElementsBytes.begin(), numElementsBytes.end());
  file.write(numElementsBytes.data(), 2);

  // Writing the description
  file.write(fillString(description, descriptionByteSize).c_str(), descriptionByteSize);

  // Looping through the list of elements in the file
  for (auto &element : elementsMetadata) {
    // Writing the filename
    file.write(fillString(element.elementFilename, fileNameByteSize).c_str(), fileNameByteSize);
    // Writing the element description
    file.write(fillString(element.elementDescription, descriptionByteSize).c_str(),
               descriptionByteSize);

    // Writing the number of channels
    std::array<char, 2> numHapticChannelsBytes{};
    memcpy(&numHapticChannelsBytes, &element.numHapticChannels, sizeof(element.numHapticChannels));
    std::reverse(numHapticChannelsBytes.begin(), numHapticChannelsBytes.end());
    file.write(numHapticChannelsBytes.data(), 2);

    // Looping through the list of channels in the element
    for (auto &channel : element.channelsMetadata) {
      // Writing the channel description
      file.write(fillString(channel.channelDescription, descriptionByteSize).c_str(),
                 descriptionByteSize);

      // Writing the channel gain
      std::array<char, 4> gainBytes{};
      memcpy(&gainBytes, &channel.gain, sizeof(channel.gain));
      file.write(gainBytes.data(), 4);

      // Writing the channel mask
      std::array<char, 4> bodyPartMaskBytes{};
      memcpy(&bodyPartMaskBytes, &channel.bodyPartMask, sizeof(channel.bodyPartMask));
      std::reverse(bodyPartMaskBytes.begin(), bodyPartMaskBytes.end());
      file.write(bodyPartMaskBytes.data(), 4);
    }
  }
  file.close();
  return true;
}

[[nodiscard]] auto OHMData::getVersion() const -> short { return version; }

auto OHMData::setVersion(short newVersion) -> void { version = newVersion; }

[[nodiscard]] auto OHMData::getNumElements() const -> short { return numElements; }

[[nodiscard]] auto OHMData::getHeader() const -> std::string { return header; }

auto OHMData::setHeader(const std::string &newHeader) -> void { header = newHeader; }

[[nodiscard]] auto OHMData::getDescription() const -> std::string { return description; }

auto OHMData::setDescription(const std::string &newDescription) -> void {
  description = newDescription;
}

auto OHMData::getHapticElementMetadataSize() -> size_t { return elementsMetadata.size(); }

auto OHMData::getHapticElementMetadataAt(int index) -> HapticElementMetadata & {
  return elementsMetadata.at(index);
}

auto OHMData::addHapticElementMetadata(HapticElementMetadata &newElementMetadata) -> void {
  elementsMetadata.push_back(newElementMetadata);
  numElements = static_cast<short>(elementsMetadata.size());
}
} // namespace haptics::tools

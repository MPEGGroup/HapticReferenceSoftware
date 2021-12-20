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

#include "../include/OHMData.h"
#include <iostream>
#include <fstream>

namespace haptics::tools {
	OHMData::OHMData(const std::string filePath) {
		loadFile(filePath);
	}

	auto OHMData::loadFile(const std::string filePath) -> void {
        std::ifstream file(filePath, std::ios::binary | std::ifstream::in);
        if (!file)
        {
            std::cout << filePath << ": Cannot open file!" << std::endl;
            file.close();
            return;
        }

        file.seekg(0, file.end);
        unsigned int length = static_cast<unsigned int>(file.tellg());
        file.seekg(0, file.beg);


        std::cout << "Open: " << length << std::endl;
        if (length == 0) { // avoid undefined behavior 
            file.close();
            return;
        }

        // Get the header and check its value
        char headerBytes[4];
        file.read(headerBytes, 4);
        header = std::string(headerBytes, 4);
        if (header != "OHM ")
        {
            std::cout << "Incorrect header: " << header << std::endl;
            file.close();
            return;
        }

        // Get the OHM version
        char versionBytes[2];
        file.read(versionBytes, 2);
        version = ((versionBytes[0] << 8) | versionBytes[1]);
        std::cout << "Version: " << version << std::endl;

        // Get the number of elements
        char numElementsBytes[2];
        file.read(numElementsBytes, 2);
        numElements = ((numElementsBytes[0] << 8) | numElementsBytes[1]);
        std::cout << "Number of elements: " << numElements << std::endl;

        // Get the file description
        char descriptionBytes[32];
        file.read(descriptionBytes, 32);
        description = std::string(descriptionBytes, 32);
        description.erase(description.find_last_not_of('\x00') + 1);

        for (int i = 0; i < numElements; i++)
        {
            HapticElementMetadata element;
            char hapticObjectFileNameBytes[64];
            file.read(hapticObjectFileNameBytes, 64);
            element.elementFilename = std::string(hapticObjectFileNameBytes, 64);
            element.elementFilename.erase(element.elementFilename.find_last_not_of('\x00') + 1);
            std::cout << "Filename: " << element.elementFilename << std::endl;

            char elementDescriptionBytes[32];
            file.read(elementDescriptionBytes, 32);
            element.elementDescription = std::string(elementDescriptionBytes, 32);
            element.elementDescription.erase(element.elementDescription.find_last_not_of('\x00') + 1);
            std::cout << "Element description: " << element.elementDescription << std::endl;

            char numHapticChannelsBytes[2];
            file.read(numHapticChannelsBytes, 2);
            element.numHapticChannels = ((numHapticChannelsBytes[0] << 8) | numHapticChannelsBytes[1]);
            std::cout << "Number of haptic channels: " << element.numHapticChannels << std::endl;

            for (int j = 0; j < element.numHapticChannels; j++)
            {
                HapticChannelMetadata channel;
                char channelDescriptionBytes[32];
                file.read(channelDescriptionBytes, 32);
                channel.channelDescription = std::string(channelDescriptionBytes, 32);
                channel.channelDescription.erase(channel.channelDescription.find_last_not_of('\x00') + 1);
                std::cout << "Channel description: " << channel.channelDescription << std::endl;

                char gainBytes[4];
                file.read(gainBytes, 4);
                channel.gain = 0;
                memcpy(&channel.gain, &gainBytes, sizeof(channel.gain));
                std::cout << "Channel gain: " << channel.gain << std::endl;

                char bodyPartMaskBytes[4];
                file.read(bodyPartMaskBytes, 4);
                channel.bodyPartMask = static_cast<Body>((bodyPartMaskBytes[0] << 24) | (bodyPartMaskBytes[1] << 16) | (bodyPartMaskBytes[2] << 8) | bodyPartMaskBytes[3]);
                //std::cout << "Channel body part mask: " << channel.bodyPartMask << std::endl;
                
                element.channelsMetadata.push_back(channel);
            }
            elementsMetadata.push_back(element);
        }
        file.close();
	}

    auto OHMData::fillString(const std::string text, const unsigned int numCharacters)->std::string
    {
        std::string res = text.length() <= numCharacters ? text : text.substr(0, numCharacters);
        unsigned int numPad = numCharacters - static_cast<unsigned int>(res.length());
        res.append(numPad,'\x00');
        return res;
    }

	auto OHMData::writeFile(const std::string filePath) -> void {
        std::ofstream file(filePath, std::ios::out | std::ios::binary);
        if (!file)
        {
            std::cout << filePath << ": Cannot open file!" << std::endl;
            return;
        }
        // Writing the header
        file.write(header.c_str(), 4);

        // Writing the version
        char versionBytes[2];
        versionBytes[0] = ((char*)&version)[1];
        versionBytes[1] = ((char*)&version)[0];
        file.write(versionBytes, 2);

        // Writing the number of elements
        char numElementsBytes[2];
        numElementsBytes[0] = ((char*)&numElements)[1];
        numElementsBytes[1] = ((char*)&numElements)[0];
        file.write(numElementsBytes, 2);

        // Writing the description
        file.write(fillString(description, 32).c_str(), 32);
        
        // Looping through the list of elements in the file
        for (auto& element : elementsMetadata) {
            // Writing the filename
            file.write(fillString(element.elementFilename, 64).c_str(), 64);
            // Writing the element description
            file.write(fillString(element.elementDescription, 32).c_str(), 32);

            // Writing the number of channels
            char numHapticChannelsBytes[2];
            numHapticChannelsBytes[0] = ((char*)&element.numHapticChannels)[1];
            numHapticChannelsBytes[1] = ((char*)&element.numHapticChannels)[0];
            file.write(numHapticChannelsBytes, 2);

            // Looping through the list of channels in the element
            for (auto& channel : element.channelsMetadata) {
                // Writing the channel description
                file.write(fillString(channel.channelDescription, 32).c_str(), 32);

                // Writing the channel gain
                char gainBytes[4];
                memcpy(&gainBytes, &channel.gain, sizeof(channel.gain));
                file.write(gainBytes, 4);

                // Writing the channel mask
                char bodyPartMaskBytes[4];
                bodyPartMaskBytes[0] = ((char*)&channel.bodyPartMask)[3];
                bodyPartMaskBytes[1] = ((char*)&channel.bodyPartMask)[2];
                bodyPartMaskBytes[2] = ((char*)&channel.bodyPartMask)[1];
                bodyPartMaskBytes[3] = ((char*)&channel.bodyPartMask)[0];
                file.write(bodyPartMaskBytes, 4);
            }

        }   
        file.close();
	}

    [[nodiscard]] auto OHMData::getVersion() const -> short {
        return version;
    }

    auto OHMData::setVersion(short newVersion) -> void {
        version = newVersion;
    }

    [[nodiscard]] auto OHMData::getNumElements() const -> short {
        return numElements;
    }
    auto OHMData::setNumElements(short newNumElements) -> void {
        numElements = newNumElements;
    }

    [[nodiscard]] auto OHMData::getDescription() const -> std::string {
        return description;
    }

    auto OHMData::setDescription(std::string &newDescription) -> void {
      description = newDescription;
    }

    auto OHMData::getHapticElementMetadataSize() -> size_t {
        return elementsMetadata.size();
    }

    auto OHMData::getHapticElementMetadataAt(int index) -> HapticElementMetadata & {
      return elementsMetadata.at(index);
    }

    auto OHMData::addHapticElementMetadata(HapticElementMetadata &newElementMetadata) -> void {
      elementsMetadata.push_back(newElementMetadata);
    }
}
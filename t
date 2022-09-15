[1mdiff --git a/source/IOHaptics/src/IOBinary.cpp b/source/IOHaptics/src/IOBinary.cpp[m
[1mindex 43ff9e9..ead8696 100644[m
[1m--- a/source/IOHaptics/src/IOBinary.cpp[m
[1m+++ b/source/IOHaptics/src/IOBinary.cpp[m
[36m@@ -547,7 +547,7 @@[m [mauto IOBinary::readTracksHeader(types::Perception &perception, std::ifstream &fi[m
       std::vector<types::BodyPartTarget> bodyPartTarget;[m
       for (uint8_t i = 0; i < bodyPartTargetCount; i++) {[m
         auto target =[m
[31m-            static_cast<types::BodyPartTarget>(IOBinaryPrimitives::readNBytes<int8_t, 1>(file));[m
[32m+[m[32m            static_cast<types::BodyPartTarget>(IOBinaryPrimitives::readNBytes<uint8_t, 1>(file));[m
         bodyPartTarget.push_back(target);[m
       }[m
 [m
[36m@@ -661,7 +661,7 @@[m [mauto IOBinary::writeTracksHeader(types::Perception &perception, std::ofstream &f[m
       auto bodyPartTargetCount = static_cast<uint8_t>(bodyPartTarget.size());[m
       IOBinaryPrimitives::writeNBytes<uint8_t, 1>(bodyPartTargetCount, file);[m
       for (uint8_t i = 0; i < bodyPartTargetCount; i++) {[m
[31m-        IOBinaryPrimitives::writeNBytes<int8_t, 1>(static_cast<int8_t>(bodyPartTarget[i]), file);[m
[32m+[m[32m        IOBinaryPrimitives::writeNBytes<uint8_t, 1>(static_cast<uint8_t>(bodyPartTarget[i]), file);[m
       }[m
 [m
       std::vector<types::Vector> actuatorTarget =[m
[1mdiff --git a/source/IOHaptics/src/IOJson.cpp b/source/IOHaptics/src/IOJson.cpp[m
[1mindex addc60b..0d5fc65 100644[m
[1m--- a/source/IOHaptics/src/IOJson.cpp[m
[1m+++ b/source/IOHaptics/src/IOJson.cpp[m
[36m@@ -238,6 +238,7 @@[m [mauto IOJson::loadTracks(const nlohmann::json &jsonTracks, types::Perception &per[m
       auto jsonBodyPartTarget = jsonTrack["body_part_target"];[m
       std::vector<types::BodyPartTarget> bodyPartTarget;[m
       for (auto itv = jsonBodyPartTarget.begin(); itv != jsonBodyPartTarget.end(); ++itv) {[m
[32m+[m[32m        auto tmp = itv.value();[m
         if (itv.value().is_string()) {[m
           bodyPartTarget.push_back([m
               types::stringToBodyPartTarget.at(itv.value().get<std::string>()));[m
[36m@@ -660,7 +661,7 @@[m [mauto IOJson::extractTracks(types::Perception &perception, nlohmann::json &jsonTr[m
       auto jsonBodyPartTarget = json::array();[m
       std::vector<types::BodyPartTarget> bodyPartTargetList = track.getBodyPartTarget().value();[m
       for (types::BodyPartTarget &bodyPartTarget : bodyPartTargetList) {[m
[31m-        jsonBodyPartTarget.push_back(bodyPartTarget);[m
[32m+[m[32m        jsonBodyPartTarget.push_back(types::bodyPartTargetToString.at(bodyPartTarget));[m
       }[m
       jsonTrack["body_part_target"] = jsonBodyPartTarget;[m
     }[m
[1mdiff --git a/source/IOHaptics/test/IOJson.test.cpp b/source/IOHaptics/test/IOJson.test.cpp[m
[1mindex b749196..2a007bb 100644[m
[1m--- a/source/IOHaptics/test/IOJson.test.cpp[m
[1m+++ b/source/IOHaptics/test/IOJson.test.cpp[m
[36m@@ -363,6 +363,93 @@[m [mTEST_CASE("write/read gmpg haptic file for track testing") {[m
   }[m
 }[m
 [m
[32m+[m[32m// NOLINTNEXTLINE(readability-function-cognitive-complexity, readability-function-size)[m
[32m+[m[32mTEST_CASE("write/read gmpg haptic file for body targetting testing") {[m
[32m+[m[32m  const std::string testingVersion = "1";[m
[32m+[m[32m  const std::string testingDate = "Thursday, September 15, 2022";[m
[32m+[m[32m  const std::string testingDescription = "Test Description";[m
[32m+[m[32m  haptics::types::Haptics testingHaptic(testingVersion, testingDate, testingDescription);[m
[32m+[m
[32m+[m[32m  const int testingId_perception = 0;[m
[32m+[m[32m  const int testingAvatarId_perception = 0;[m
[32m+[m[32m  const std::string testingDescription_perception = "I'm just a random string to fill the place";[m
[32m+[m[32m  const auto testingPerceptionModality_perception =[m
[32m+[m[32m      haptics::types::PerceptionModality::Vibrotactile;[m
[32m+[m[32m  const int testingId_track = 0;[m
[32m+[m[32m  const std::string testingDescription_track = "testingDescription_track0";[m
[32m+[m[32m  const float testingGain_track = .34;[m
[32m+[m[32m  const float testingMixingWeight_track = 1;[m
[32m+[m[32m  const uint32_t testingBodyPartMask_track = 32;[m
[32m+[m[32m  haptics::types::Perception testingPerception(testingId_perception, testingAvatarId_perception,[m
[32m+[m[32m                                                testingDescription_perception,[m
[32m+[m[32m                                                testingPerceptionModality_perception);[m
[32m+[m[32m  haptics::types::Track testingTrack0(testingId_track, testingDescription_track,[m
[32m+[m[32m                                      testingGain_track, testingMixingWeight_track,[m
[32m+[m[32m                                      testingBodyPartMask_track);[m
[32m+[m[32m  haptics::types::Track testingTrack1(testingId_track, testingDescription_track,[m
[32m+[m[32m                                      testingGain_track, testingMixingWeight_track,[m
[32m+[m[32m                                      testingBodyPartMask_track);[m
[32m+[m[41m  [m
[32m+[m[32m  const haptics::types::Vector testingTrackResolution_track0(32, 110, 3);[m
[32m+[m[32m  const std::vector<haptics::types::Vector> testingActuatorTarget_track0 {[m
[32m+[m[32m      haptics::types::Vector{31, 109, 2},[m
[32m+[m[32m      haptics::types::Vector{0, 0, 0},[m
[32m+[m[32m      haptics::types::Vector{15, 42, 1},[m
[32m+[m[32m  };[m
[32m+[m[32m  testingTrack0.setTrackResolution(testingTrackResolution_track0);[m
[32m+[m[32m  testingTrack0.setActuatorTarget(testingActuatorTarget_track0);[m
[32m+[m
[32m+[m[32m  const std::vector<haptics::types::BodyPartTarget> testingBodyPartTarget_track1{[m
[32m+[m[32m      haptics::types::BodyPartTarget::Left,[m
[32m+[m[32m      haptics::types::BodyPartTarget::Index,[m
[32m+[m[32m      haptics::types::BodyPartTarget::ThirdPhalanx,[m
[32m+[m[32m      haptics::types::BodyPartTarget::Plus,[m
[32m+[m[32m      haptics::types::BodyPartTarget::Right,[m
[32m+[m[32m      haptics::types::BodyPartTarget::Leg,[m
[32m+[m[32m      haptics::types::BodyPartTarget::Minus,[m
[32m+[m[32m      haptics::types::BodyPartTarget::Hallux,[m
[32m+[m[32m  };[m
[32m+[m[32m  testingTrack1.setBodyPartTarget(testingBodyPartTarget_track1);[m
[32m+[m
[32m+[m[32m  testingPerception.addTrack(testingTrack0);[m
[32m+[m[32m  testingPerception.addTrack(testingTrack1);[m
[32m+[m[32m  testingHaptic.addPerception(testingPerception);[m
[32m+[m
[32m+[m[32m  SECTION("write haptic file") {[m
[32m+[m[32m    IOJson::writeFile(testingHaptic, filename);[m
[32m+[m[32m    CHECK(std::filesystem::is_regular_file(filename));[m
[32m+[m[32m  }[m
[32m+[m
[32m+[m[32m  SECTION("read haptic file") {[m
[32m+[m[32m    haptics::types::Haptics res;[m
[32m+[m[32m    bool succeed = IOJson::loadFile(filename, res);[m
[32m+[m[32m    REQUIRE(succeed);[m
[32m+[m[32m    REQUIRE(res.getPerceptionsSize() == 1);[m
[32m+[m[32m    REQUIRE(res.getPerceptionAt(0).getTracksSize() == 2);[m
[32m+[m[32m    haptics::types::Track res_track0 = res.getPerceptionAt(0).getTrackAt(0);[m
[32m+[m[32m    haptics::types::Track res_track1 = res.getPerceptionAt(0).getTrackAt(1);[m
[32m+[m
[32m+[m[32m    REQUIRE(res_track0.getTrackResolution().has_value());[m
[32m+[m[32m    CHECK(res_track0.getTrackResolution().value() == testingTrackResolution_track0);[m
[32m+[m[32m    REQUIRE(res_track0.getActuatorTarget().has_value());[m
[32m+[m[32m    REQUIRE(res_track0.getActuatorTarget().value().size() == testingActuatorTarget_track0.size());[m
[32m+[m[32m    for (size_t i = 0; i < testingActuatorTarget_track0.size(); i++) {[m
[32m+[m[32m      CHECK(res_track0.getActuatorTarget().value()[i] == testingActuatorTarget_track0[i]);[m
[32m+[m[32m    }[m
[32m+[m[32m    CHECK_FALSE(res_track0.getBodyPartTarget().has_value());[m
[32m+[m[32m    CHECK_FALSE(res_track1.getTrackResolution().has_value());[m
[32m+[m[32m    CHECK_FALSE(res_track1.getActuatorTarget().has_value());[m
[32m+[m[32m    REQUIRE(res_track1.getBodyPartTarget().has_value());[m
[32m+[m[32m    REQUIRE(res_track1.getBodyPartTarget().value().size() == testingBodyPartTarget_track1.size());[m
[32m+[m[32m    for (size_t i = 0; i < testingBodyPartTarget_track1.size(); i++) {[m
[32m+[m[32m      CHECK(res_track1.getBodyPartTarget().value()[i] == testingBodyPartTarget_track1[i]);[m
[32m+[m[32m    }[m
[32m+[m
[32m+[m[32m    std::filesystem::remove(filename);[m
[32m+[m[32m    CHECK(!std::filesystem::is_regular_file(filename));[m
[32m+[m[32m  }[m
[32m+[m[32m}[m
[32m+[m
 // NOLINTNEXTLINE(readability-function-cognitive-complexity, readability-function-size)[m
 TEST_CASE("write/read gmpg haptic file for signal testing") {[m
   const std::string testingVersion = "1";[m
[1mdiff --git a/source/Types/include/BodyPartTarget.h b/source/Types/include/BodyPartTarget.h[m
[1mindex f82c93b..78b2509 100644[m
[1m--- a/source/Types/include/BodyPartTarget.h[m
[1m+++ b/source/Types/include/BodyPartTarget.h[m
[36m@@ -54,7 +54,7 @@[m [menum class BodyPartTarget : uint8_t {[m
   Waist = 23,[m
   Leg = 24,[m
 [m
[31m-  Upperarm = 30,[m
[32m+[m[32m  UpperArm = 30,[m
   Forearm = 31,[m
   Hand = 32,[m
   Crane = 33,[m
[36m@@ -99,7 +99,7 @@[m [mstatic const std::map<std::string, BodyPartTarget> stringToBodyPartTarget = {[m
     {"Chest", BodyPartTarget::Chest},[m
     {"Waist", BodyPartTarget::Waist},[m
     {"Leg", BodyPartTarget::Leg},[m
[31m-    {"Upperarm", BodyPartTarget::Upperarm},[m
[32m+[m[32m    {"Upper-arm", BodyPartTarget::UpperArm},[m
     {"Forearm", BodyPartTarget::Forearm},[m
     {"Hand", BodyPartTarget::Hand},[m
     {"Crane", BodyPartTarget::Crane},[m
[36m@@ -117,10 +117,10 @@[m [mstatic const std::map<std::string, BodyPartTarget> stringToBodyPartTarget = {[m
     {"Ring", BodyPartTarget::Ring},[m
     {"Pinky", BodyPartTarget::Pinky},[m
     {"Hallux", BodyPartTarget::Hallux},[m
[31m-    {"Index Toe", BodyPartTarget::IndexToe},[m
[31m-    {"Middle Toe", BodyPartTarget::MiddleToe},[m
[31m-    {"Ring Toe", BodyPartTarget::RingToe},[m
[31m-    {"Pinky Toe", BodyPartTarget::PinkyToe},[m
[32m+[m[32m    {"Index-toe", BodyPartTarget::IndexToe},[m
[32m+[m[32m    {"Middle-toe", BodyPartTarget::MiddleToe},[m
[32m+[m[32m    {"Ring-toe", BodyPartTarget::RingToe},[m
[32m+[m[32m    {"Pinky-toe", BodyPartTarget::PinkyToe},[m
     {"First Phalanx", BodyPartTarget::FirstPhalanx},[m
     {"Second Phalanx", BodyPartTarget::SecondPhalanx},[m
     {"Third Phalanx", BodyPartTarget::ThirdPhalanx},[m
[36m@@ -138,7 +138,7 @@[m [mstatic const std::map<BodyPartTarget, std::string> bodyPartTargetToString = {[m
     {BodyPartTarget::Chest, "Chest"},[m
     {BodyPartTarget::Waist, "Waist"},[m
     {BodyPartTarget::Leg, "Leg"},[m
[31m-    {BodyPartTarget::Upperarm, "Upperarm"},[m
[32m+[m[32m    {BodyPartTarget::UpperArm, "Upper-arm"},[m
     {BodyPartTarget::Forearm, "Forearm"},[m
     {BodyPartTarget::Hand, "Hand"},[m
     {BodyPartTarget::Crane, "Crane"},[m
[36m@@ -156,10 +156,10 @@[m [mstatic const std::map<BodyPartTarget, std::string> bodyPartTargetToString = {[m
     {BodyPartTarget::Ring, "Ring"},[m
     {BodyPartTarget::Pinky, "Pinky"},[m
     {BodyPartTarget::Hallux, "Hallux"},[m
[31m-    {BodyPartTarget::IndexToe, "Index Toe"},[m
[31m-    {BodyPartTarget::MiddleToe, "Middle Toe"},[m
[31m-    {BodyPartTarget::RingToe, "Ring Toe"},[m
[31m-    {BodyPartTarget::PinkyToe, "Pinky Toe"},[m
[32m+[m[32m    {BodyPartTarget::IndexToe, "Index-toe"},[m
[32m+[m[32m    {BodyPartTarget::MiddleToe, "Middle-toe"},[m
[32m+[m[32m    {BodyPartTarget::RingToe, "Ring-toe"},[m
[32m+[m[32m    {BodyPartTarget::PinkyToe, "Pinky-toe"},[m
     {BodyPartTarget::FirstPhalanx, "First Phalanx"},[m
     {BodyPartTarget::SecondPhalanx, "Second Phalanx"},[m
     {BodyPartTarget::ThirdPhalanx, "Third Phalanx"},[m

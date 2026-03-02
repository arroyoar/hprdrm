#include <gtest/gtest.h>
#include "AudioEngine.h"

// Test that the AudioEngine initializes its values to zero
TEST(AudioEngineTest, DefaultInitialization) {
    AudioEngine audio;
    EXPECT_FLOAT_EQ(audio.getSubBass(), 0.0f);
    EXPECT_FLOAT_EQ(audio.getBass(), 0.0f);
    EXPECT_FLOAT_EQ(audio.getLowMid(), 0.0f);
    EXPECT_FLOAT_EQ(audio.getMid(), 0.0f);
    EXPECT_FLOAT_EQ(audio.getHighMid(), 0.0f);
    EXPECT_FLOAT_EQ(audio.getPresence(), 0.0f);
    EXPECT_FLOAT_EQ(audio.getTreble(), 0.0f);

    EXPECT_FLOAT_EQ(audio.getSubBassMaxDuration(), 0.0f);
    EXPECT_FLOAT_EQ(audio.getBassMaxDuration(), 0.0f);
    EXPECT_FLOAT_EQ(audio.getLowMidMaxDuration(), 0.0f);
    EXPECT_FLOAT_EQ(audio.getMidMaxDuration(), 0.0f);
    EXPECT_FLOAT_EQ(audio.getHighMidMaxDuration(), 0.0f);
    EXPECT_FLOAT_EQ(audio.getPresenceMaxDuration(), 0.0f);
    EXPECT_FLOAT_EQ(audio.getTrebleMaxDuration(), 0.0f);
}

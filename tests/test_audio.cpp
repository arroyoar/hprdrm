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

// Test playback state management
TEST(AudioEngineTest, PlaybackStateTracking) {
    AudioEngine audio;
    
    // Before initialization, nothing should be playing
    EXPECT_FALSE(audio.isPlaying());
    EXPECT_EQ(audio.getCurrentTrackName(), "No Track Loaded");
    
    // We cannot easily test loadTrack() or loadDirectory() in a unit test without 
    // guaranteeing a valid audio file exists on the filesystem where the test runs.
    // However, we can verify that attempting to toggle pause without a track does not crash.
    EXPECT_NO_THROW(audio.togglePause());
    EXPECT_NO_THROW(audio.nextTrack());
    EXPECT_NO_THROW(audio.prevTrack());
    EXPECT_NO_THROW(audio.seekForward(5.0f));
}

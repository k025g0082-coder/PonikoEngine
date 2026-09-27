#pragma once

#include <cassert>
#include <map>
#include <string>

#include <xaudio2.h>
#pragma comment(lib, "xaudio2.lib")

#include <fstream>
#include <vector>

//ここから音用

struct ChunkHeader {
	char id[4];//チャンク毎のID
	int32_t size;// チャンクサイズ
};

// RIFFヘッダチャンク
struct RiffHeader {
	ChunkHeader chunk;	// "RIFF"
	char type[4];// "WAVE"

};

struct FormatChunk {
	ChunkHeader chunk;
	WAVEFORMATEX fmt;
};

// 音声データ
struct SoundData {

	//波形フォーマット
	WAVEFORMATEX wfex;
	//バッファの先頭アドレス
	BYTE* pBuffer;
	// バッファのサイズ
	unsigned int bufferSize;

	// ループ
	bool isLoop = false;

	// 範囲ループするか
	bool isRangeLoop = false;

	float rangeStart;
	float rangeLength;

	bool isFadingOut;
	float fadeTimer;
	float fadeTime;

	IXAudio2SourceVoice* sourceVoice = nullptr;
};

class SoundManager {
public:

	// シングルトン
	static SoundManager* GetInstance();

	void Initialize();

	// 毎回更新
	void Update();
	
	// wavファイル読込
	void LoadWave(const std::string& name, const char* filename);

	// 再生
	void Play(const std::string& name, float playSpeed = 1.0f, float startTime = 0.0f);

	// 一時停止
	void StopSound(const std::string& name);

	// ループとループ範囲を設定
	void SetSoundLoop(const std::string& name);
	void SetSoundLoop(const std::string& name, float loopStart, float loopEnd);

	// フェーディング設定
	void FadeOut(const std::string& name, float fadeTime);

	// 左右のパン
	void SetSoundPan(const std::string& name, float pan);
	
	void Finalize();

	// 全部停止
	void AllStopSound();

	// マスターの音量
	void SetMasterSoundAmp(float n = 1.0f) {
		if (masterVoice) {
			masterVoice->SetVolume(n);
		}
	}

private:

	// 音
	IXAudio2* xAudio2 = nullptr;
	IXAudio2MasteringVoice* masterVoice;

	std::map<std::string, SoundData> sounds_;

	std::vector<IXAudio2SourceVoice*> pSourceVoices;

	// 読込
	SoundData SoundLoadWave(const char* filename);

	// 廃棄
	void SoundUnload(SoundData* soundData);

	// 再生
	void SoundPlayWave(IXAudio2* xAudio2, SoundData& soundData, float startTime, float playSpeed);

};


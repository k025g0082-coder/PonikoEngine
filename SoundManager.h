#pragma once

#include <cassert>
#include <map>
#include <string>

#include <xaudio2.h>
#pragma comment(lib, "xaudio2.lib")

#include <fstream>

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

};

class SoundManager {
public:

	// シングルトン
	static SoundManager* GetInstance();

	void Initialize();

	void LoadWave(const std::string& name, const char* filename);

	void Play(const std::string& name);

	void Finalize();
private:

	// 音
	IXAudio2* xAudio2 = nullptr;
	IXAudio2MasteringVoice* masterVoice;

	std::map<std::string, SoundData> sounds_;


	// 読込
	SoundData SoundLoadWave(const char* filename);

	// 廃棄
	void SoundUnload(SoundData* soundData);

	// 再生
	void SoundPlayWave(IXAudio2* xAudio2, const SoundData& soundData);
};


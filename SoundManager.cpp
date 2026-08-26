#include "SoundManager.h"

// シングルトン
SoundManager* SoundManager::GetInstance() {
	static SoundManager instance;
	return &instance;
}

void SoundManager::Initialize() {

	HRESULT hr = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(hr));

	hr = xAudio2->CreateMasteringVoice(&masterVoice);
	assert(SUCCEEDED(hr));
};

void SoundManager::LoadWave(const std::string& name, const char* filename) {

	sounds_[name] = SoundLoadWave(filename);
}

void SoundManager::Play(const std::string& name) {

	auto it = sounds_.find(name);

	if (it == sounds_.end()) {
		return;
	}


	const SoundData& soundData = it->second;

	
	SoundPlayWave(xAudio2, soundData);
}
void SoundManager::Finalize() {

	for (auto& sound : sounds_) {

		SoundUnload(&sound.second);
	}

	sounds_.clear();

	if (masterVoice) {
		masterVoice->DestroyVoice();
		masterVoice = nullptr;
	}

	if (xAudio2) {
		xAudio2->Release();
		xAudio2 = nullptr;
	}
}

SoundData SoundManager::SoundLoadWave(const char* filename)
{

	//HRESULT result;

	//	1ファイルオープン

	// ファイル入力ストリームのインスタンス
	std::ifstream file;
	//.wavファイルをバイナリモードで開く
	file.open(filename, std::ios_base::binary);
	// ファイルオープン失敗を検出する
	assert(file.is_open());

	//	2.wavデータ読み込み

	//RIFFヘッダーの読み込み
	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));
	// ファイルがRIFFかチェック
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
		assert(0);
	}
	// タイプがWAVEかチェック
	if (strncmp(riff.type, "WAVE", 4) != 0) {
		assert(0);
	}
	// Formatチャンクの読み込み
	FormatChunk format = {};

	// fmtチャンクを探す
	while (file.read(
		reinterpret_cast<char*>(&format.chunk),
		sizeof(ChunkHeader))) {

		if (strncmp(format.chunk.id, "fmt ", 4) == 0) {
			break;
		}

		file.seekg(format.chunk.size, std::ios_base::cur);
	}

	assert(strncmp(format.chunk.id, "fmt ", 4) == 0);

	// fmtチャンク本体
	assert(format.chunk.size <= sizeof(format.fmt));

	file.read(
		reinterpret_cast<char*>(&format.fmt),
		format.chunk.size
	);


	// Dataチャンクを探す
	ChunkHeader data{};

	while (file.read(reinterpret_cast<char*>(&data), sizeof(data))) {

		// dataチャンクなら終了
		if (strncmp(data.id, "data", 4) == 0) {
			break;
		}

		// data以外のチャンクは読み飛ばす
		file.seekg(data.size, std::ios_base::cur);
	}

	// dataチャンクが見つからなかった
	assert(strncmp(data.id, "data", 4) == 0);

	//Dataチャンクのデータ部(波形データ)の読み込み
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);

	//	3ファイルクローズ
	file.close();

	//	4読み込んだ音声データをreturn
	SoundData soundData = {};

	soundData.wfex = format.fmt;
	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.bufferSize = data.size;

	return soundData;
}

// 音声データ開放
void SoundManager::SoundUnload(SoundData* soundData) {

	//バッファのメモリを解放
	delete[] soundData->pBuffer;

	soundData->pBuffer = 0;
	soundData->bufferSize = 0;
	soundData->wfex = {};

}

// 再生
void SoundManager::SoundPlayWave(IXAudio2* xAudio2, const SoundData& soundData) {

	HRESULT result;

	// 波形フォーマットを元にSourceVoiceの生成
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result));

	// 再生する波形データの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;

	// 波形データの再生
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->Start();
};

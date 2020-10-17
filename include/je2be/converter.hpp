#pragma once

namespace j2e {

class Converter {
public:
	class InputOption {
	public:
	};

	class OutputOption {
	public:
	};

	Converter(std::string const& input, InputOption io, std::string const& output, OutputOption oo)
		: fInput(input)
		, fOutput(output)
		, fInputOption(io)
		, fOutputOption(oo)
	{}

	bool run(unsigned int concurrency) {
		using namespace std;
		namespace fs = mcfile::detail::filesystem;

		auto rootPath = fs::path(fOutput);
		auto dbPath = rootPath / "db";

		fs::create_directory(rootPath);
		fs::create_directory(dbPath);
		LevelData levelData;
		levelData.write(fOutput + string("/level.dat"));

		Db db(dbPath.string());
		if (!db.valid()) {
			return false;
		}

		return true;
	}

private:
	std::string const fInput;
	std::string const fOutput;
	InputOption const fInputOption;
	OutputOption const fOutputOption;
};

}

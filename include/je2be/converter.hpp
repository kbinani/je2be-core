#pragma once

namespace j2e {

class Converter {
public:
	class InputOption {
	public:
		LevelDirectoryStructure fLevelDirectoryStructure = LevelDirectoryStructure::Vanilla;

		std::string getWorldDirectory(std::string const& root, Dimension dim) const {
			switch (fLevelDirectoryStructure) {
			case LevelDirectoryStructure::Vanilla: {
				switch (dim) {
				case Dimension::Overworld:
					return root;
				case Dimension::Nether:
					return root + "/DIM-1";
				case Dimension::End:
					return root + "/DIM1";
				}
				break;
			}
			case LevelDirectoryStructure::Paper: {
				switch (dim) {
				case Dimension::Overworld:
					return root + "/world";
				case Dimension::Nether:
					return root + "/world_nether/DIM-1";
				case Dimension::End:
					return root + "/world_the_end/DIM1";
				}
				break;
			}
			}
		}
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
		using namespace mcfile;

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

		bool ok = true;
		for (auto dim : { Dimension::Overworld, Dimension::Nether, Dimension::End }) {
			auto dir = fInputOption.getWorldDirectory(fInput, dim);
			World world(dir);
			ok &= convertWorld(world, dim);
		}

		return ok;
	}

private:
	bool convertWorld(mcfile::World const& w, Dimension dim) {
		//TODO:
		return false;
	}

private:
	std::string const fInput;
	std::string const fOutput;
	InputOption const fInputOption;
	OutputOption const fOutputOption;
};

}

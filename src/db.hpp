#pragma once

namespace j2e {

class Db {
public:
	explicit Db(std::string const& dir)
		: fDb(nullptr)
		, fValid(false)
	{
		using namespace leveldb;

	    DB *db;
	    Options options;
	    options.compression = kZlibCompression;
	    options.create_if_missing = true;
	    Status status = DB::Open(options, dir.c_str(), &db);
	    if (!status.ok()) {
	    	return;
	    }
	    fDb = db;
	    fValid = true;
	}

	~Db() {
		if (fValid) {
			delete fDb;
		}
	}

	bool valid() const {
		return fValid;
	}

	void put(std::string const& key, leveldb::Slice const& value) {
		assert(fValid);
		fDb->Put(fWriteOptions, key, value);
	}

private:
	leveldb::DB *fDb;
	bool fValid;
	leveldb::WriteOptions fWriteOptions;
};

}

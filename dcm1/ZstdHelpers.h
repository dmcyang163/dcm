#pragma once
class CZstdHelpers
{
public:
	CZstdHelpers();
	~CZstdHelpers();

	static void compress_orDie(const char* fname, const char* oname);
	static char* createOutFilename_orDie(const char* filename);

};



#if 0

int main(int argc, const char** argv)
{
	const char* const exeName = argv[0];

	if (argc != 2) {
		printf("wrong arguments\n");
		printf("usage:\n");
		printf("%s FILE\n", exeName);
		return 1;
	}

	const char* const inFilename = argv[1];

	char* const outFilename = createOutFilename_orDie(inFilename);
	compress_orDie(inFilename, outFilename);
	free(outFilename);
	return 0;
}

#endif
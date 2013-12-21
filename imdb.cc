using namespace std;
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "imdb.h"
#include <algorithm>
#include <string>
using namespace std;

const char *const imdb::kActorFileName = "actordata";
const char *const imdb::kMovieFileName = "moviedata";
imdb::imdb(const string& directory) {
  const string actorFileName = directory + "/" + kActorFileName;
  const string movieFileName = directory + "/" + kMovieFileName;  
  actorFile = acquireFileMap(actorFileName, actorInfo);
  movieFile = acquireFileMap(movieFileName, movieInfo);
  
  
}

bool imdb::good() const {
  return !( (actorInfo.fd == -1) || 
	    (movieInfo.fd == -1) ); 
}

imdb::~imdb() {
  releaseFileMap(actorInfo);
  releaseFileMap(movieInfo);
}

//helper function to check if lower_bound returned a valid actor match.  
bool imdb::verifyActor(int *foundIndex, const string& player) const{
	int foundOffset = *foundIndex;
	char *actorNameCString = (char*)actorFile + foundOffset;  //points to the start of the actor's block
	string actorNameString(actorNameCString); //construct a C++ String from a c-string
	if(actorNameString != player) return false;	
	return true;
	
	
}
	
//helper function to check if lower_bound returned a valid movie match
bool imdb::verifyFilm(int *foundIndex, const film& movie) const{
	int foundOffset = *foundIndex;
	char *movieRecord = (char*)movieFile + foundOffset;  //points to the start of the actor's block
	film foundFilm = constructFilmStruct(movieRecord);
	if(foundFilm == movie) return true;
	return false;
	
}
	
//helper function returns a populated film struct, given a pointer to a film record
film imdb::constructFilmStruct(void *filmPtr) const{

	char *filmCstring = (char*)filmPtr;
	string filmName(filmCstring);
	int stringLength = filmName.length();
	char* yearPointer = (char*)filmPtr+stringLength+1; //+1 to skip over the null terminator
	char yearOffset= *(char*)yearPointer;  //year is kept as an offset from 1900 (1 byte long only)
	int year = 1900+(int)yearOffset;
	
	film tempFilm;
	tempFilm.title=filmName;
	tempFilm.year=year;
	return tempFilm;

}

//generic helper function for getCredits.  Will look for the given name within a given file and return a pointer to that data in the file. Returns true if it found a match for the given string, false otherwise.  Assumes file structure starts with an int that specifies the number of entities, followed by an index of offsets for said entities.  Each entity, when located, starts with a string name. 

int *imdb::returnFoundIndexActorPtr(const string& player) const {
	int numActors = *(int*)actorFile;
	int *indexEndPointer = (int*)actorFile + numActors; //past-the end pointer
	int *indexBeginPointer = (int*)actorFile+1;  //start of index

	//now Search (using lower_bound) for the index slot that should correspond to the desired actor
	int *foundIndex = lower_bound(indexBeginPointer,indexEndPointer,player,[&actorFile](const int& offset, const string& searchActorName)-> bool{
	
		char *actorNameCString = (char*)actorFile + offset;
		string actorNameString(actorNameCString); //construct a C++ String from a c-string
		if(actorNameString < searchActorName) return true;
		return false;
	});
	return foundIndex;
	
}


//generic helper function for getCast.  Will look for the given film within a given file and return a pointer to the corresponding record in the file. Returns true if it found a match for the given film struct, false otherwise.  Assumes file structure starts with an int that specifies the number of films, followed by an index of offsets for said films. 

int *imdb::returnFoundIndexMoviePtr(const film& movie) const{
	int numMovies = *(int*)movieFile;
	int *indexEndPointer = (int*)movieFile + numMovies; //past-the end pointer
	int *indexBeginPointer = (int*)movieFile+1;  //start of index
	int *foundIndex = lower_bound(indexBeginPointer,indexEndPointer,movie,[&](const int& offset, const film& searchMovie)-> bool{

		char *moviePtr = (char*)movieFile + offset;
		film filmStruct = constructFilmStruct(moviePtr);
		
		if(filmStruct < searchMovie) return true;  //film struct supports comparison operators
		return false;
	});
	return foundIndex;

}

	
bool imdb::getCredits(const string& player, vector<film>& films) const { 

	int* foundIndex = returnFoundIndexActorPtr(player);

	//now verify if this offset leads to the correct actor
	bool verified = verifyActor(foundIndex,player);
	if(verified==false){
		return false;
	}
	
	//otherwise, the person has been found!  Time to collect movie info. 
	int runningTotal=0;
	
	int stringLen = player.length();
	if(stringLen%2 ==0){ //if length of name is even, then there are two null terms afterwards, so runningTotal = stringLen +2	
		runningTotal=stringLen+2;
	}
	else{  //only 1 null term at end
		runningTotal=stringLen+1;
	}
	
	int foundOffset = *foundIndex;
	char *actorRecordPtr = (char*)actorFile+foundOffset;
	char* numberFilmsPointer = actorRecordPtr + runningTotal;  //points to the SHORT indicating # films
	short numFilms = *(short*)numberFilmsPointer;
	runningTotal=runningTotal+2;  //increase running total bytes by 2 to move past the film count SHORT
	if(runningTotal%4 !=0){
		runningTotal=runningTotal+2;  //check if multiple of 4.  If not, realign by adding 2 more bytes. 
	}
	char *actorMovieIndexStart = actorRecordPtr + runningTotal; //this points to the start of the movie index for this actor
	
	for(int i=0; i<numFilms;i++){  //add all the film structs
		
		int movieOffset = *((int *)actorMovieIndexStart+i);  //4-byte int ptr math selects the movie offset slot (increments of 4 bytes)	
		char* filmPointer= (char*)movieFile + movieOffset;
		film tempFilm = constructFilmStruct(filmPointer);	
		films.push_back(tempFilm);
	}

  return true; 
}

bool imdb::getCast(const film& movie, vector<string>& players) const { 

	int* foundIndex = returnFoundIndexMoviePtr(movie);
	
	//now verify if this offset leads to the correct movie
	bool verified = verifyFilm(foundIndex,movie);
	if(verified==false){
		cout<<movie.title<<" is not in the database."<<endl;
		return false;
	}
	
	//otherwise, the movie has been found!  Time to collect cast info. 
	int runningTotal= movie.title.length()+1+1;  //+1 for the null term +1 for 1-byte year offset
	if(runningTotal%2 !=0) runningTotal=runningTotal+1;  //if odd number, added extra null term to make it even
	
	int foundOffset = *foundIndex;
	char *movieRecordPtr = (char*)movieFile+foundOffset;
	char* numberCastPointer = movieRecordPtr + runningTotal;  //points to the SHORT indicating # actors
	int numCast = *(short *)numberCastPointer;  //defines our cast loop
	runningTotal=runningTotal+2;  //add the two bytes of the SHORT
	if(runningTotal%4!=0) runningTotal=runningTotal+2; //if not a multiple of 4 yet, it is padded with 2 more bytes to make it so
	char*castIndexStart = movieRecordPtr+runningTotal;  //find the start of the cast offset list for this movie
	
	
	for(int i=0; i<numCast;i++){  //add all the cast members names
		
		int castOffset = *((int *)castIndexStart+i);  //4-byte pointer math since movie offsets are ints
		char* castPointer= (char*)actorFile + castOffset;
		string castStringName(castPointer);
		players.push_back(castStringName);
	}

  return true;
}

const void *imdb::acquireFileMap(const string& fileName, struct fileInfo& info) {
  struct stat stats;
  stat(fileName.c_str(), &stats);
  info.fileSize = stats.st_size;
  info.fd = open(fileName.c_str(), O_RDONLY);
  return info.fileMap = mmap(0, info.fileSize, PROT_READ, MAP_SHARED, info.fd, 0);
}

void imdb::releaseFileMap(struct fileInfo& info) {
  if (info.fileMap != NULL) munmap((char *) info.fileMap, info.fileSize);
  if (info.fd != -1) close(info.fd);
}

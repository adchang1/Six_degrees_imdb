#include <vector>
#include "imdb.h"
#include "path.h"
#include <set>
#include <list>
#include <assert.h>


using namespace std;


int main(int argc, const char *argv[]) {

	//error checking on input
	if(argc!=3){
		cout<<"Usage: six-degrees <source-actor> <target-actor>"<<endl;
		return 0;
	}
	string person1(argv[1]);
	string person2(argv[2]);
	
	if(person1==person2){   //check for identical actors
		cout<<"Ensure that source and target actors are different!"<<endl;
		return 0;
	}
	imdb database(kIMDBDataDirectory);

	//check the validity of the supplied actors before even attempting to form connections
	vector<film> check;
	if(database.getCredits(person1, check) == false || database.getCredits(person2, check) == false){
		cout<<"No path between those two people could be found."<<endl;
		return 0; 
	}
	
	//initialize vars
	set<string> visitedPeople;
	set<film> visitedFilms;
	
	list<path> pathQueue;
	path startPath(person1);
	pathQueue.push_back(startPath);

	
	while(!pathQueue.empty()){
		path recentPath = pathQueue.front();  //retrieve next path from the list
		pathQueue.pop_front();   //delete it from the list
		string lastPersonInPath = recentPath.getLastPlayer();

		if(recentPath.getLength()>6){   //check if path lengths are too long (essentially no connection found)
			cout<<"No path between those two people could be found"<<endl;
			return 0;
		}
		//otherwise..continue searching
	
		vector<film>films;   //holding vector for the films of this actor
		bool successCred = database.getCredits(lastPersonInPath,films);
		assert(successCred == true);

		for(size_t i=0; i<films.size(); i++){
		
			if(visitedFilms.find(films[i])==visitedFilms.end()){  //only execute for films that could not be found in visited
				
				vector<string>cast;   //holding vector for the cast members of this film
				bool castSuccess=database.getCast(films[i],cast);
				assert(castSuccess==true);
				
				for(size_t j=0; j<cast.size();j++){
					if(visitedPeople.find(cast[j])==visitedPeople.end()){  //only execute for people that could not be found in visited
						path newPath=recentPath;
						newPath.addConnection(films[i],cast[j]);
						pathQueue.push_back(newPath);
						visitedPeople.insert(cast[j]);   //added the person to a path
						
						if((cast[j]) == person2){   //check if a connection to person2 was found!  Do this before adding a new path to improve speed
							cout<<newPath<<endl;
							return 0;
						}
					}
				}
				visitedFilms.insert(films[i]); //done with film, so add it to set of visited films
			}	
		} //end parsing over all movies of lastPlayer in the retrieved path
	}//only exits while loop if queue is empty 
	
	cout<<"No path between those two people could be found."<<endl;
  	return 0;
}

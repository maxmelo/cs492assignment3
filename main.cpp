/*
 * Assignment 3
 * Max Melo and Michael Iacona
 * April 26th, 2018
 * I pledge my honor that I have abided by the Stevens Honor System
 */

#include <stdlib.h>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <math.h>
#include <string.h>
#include <list>

using namespace std;


/* ----------------- *
 *  	CLASSES		 *
 * ----------------- */
//Class to create the disk block structure
class Block {
	public:
	bool free;
	int sid;
	int eid;

	//Constructor for a block
	Block(int start, int end, bool free) {
		sid = start;
		eid = end;
		free = free;
	}
};

//Class to create the structure for file information
//Constructor to mark the time that the file is created
//each file has a size and number of bytes
class File {
	public:
	int size; 
	bool is_dir; 
	int bytes;
	
	list<int> addrs;
	time_t timestamp;
	string name;
	//constructor for a file
	File(string n, int s, bool isD) {
		name = n;
		size = s;
		is_dir = isD;
		time(&timestamp);
		bytes = 0;
	}

	// Function to display the info associated with a file
	void display(bool full) {
		cout << "path: " << name << "     dir: " << (is_dir ? "yes" : "no") << "     size: " << size << "     time: " << ctime(&timestamp) << endl;
		
		if (full) {
			list<int>::iterator it;
			cout << "block addresses: ";
			for (it = addrs.begin(); it != addrs.end(); it++) cout << *it << ", ";
			cout << endl;
		}
	}
};

//Class to create the directed tree structure to represent a hierachical drirectory
class Tree {
	public:
	File* file;
	vector<Tree*> children; //a vector to hold child nodes
	Tree* parent;

	Tree(File* f) {
		file = f;
		parent = NULL;
	}
};
//adds children to the tree
void addChild(Tree* parent, Tree* child) {
	parent->children.push_back(child);
	child->parent = parent;
}

//A function to traverse the tree using breadth first search tree traversal
//displays the file data for each node visited
void tree_bfs(Tree* start) {
	if (start != NULL) {
		Tree* temp = start; //tracks progress for each tree traversal
		vector<Tree *> q;
		
		while (temp != NULL) { //loop until there are no more remaining nodes
			temp->file->display(false);
			
			for (int i = 0; i < temp->children.size(); i++) {
				q.push_back(temp->children[i]);
			}
			
			if (q.size() == 0) {
				temp = NULL;
			} else { //sets the temp variable to the next tree in the queue
				temp = q[0];
				q.erase(q.begin());
			}
		}
	}
}

//Return a vector of all items in the tree
vector<Tree*> traverse(Tree* dir_root) {
	Tree* current = dir_root;
	vector<Tree*> queue;
	vector<Tree*> out;
	
	while (current != NULL) {		
		for (int i = 0; i < current->children.size(); i++) queue.push_back(current->children[i]);
		
		out.push_back(current);
		
		if (queue.size() == 0) return out;
		else {
			current = queue[0];
			queue.erase(queue.begin());
		}
	}

	return out;
}


// method to find the specific file name we are looking for and retrieve the data for the node we want
Tree* find(Tree *start, string filename) {
	if (start == NULL) return NULL;

	vector<Tree*> trv = traverse(start);

	for (vector<Tree*>::iterator it = trv.begin(); it != trv.end(); it++) {
		if ((*it)->file->name == filename) return (*it);
	}

	return NULL;
}


//function to delete a child without children
//removes child from the parent's vector
Tree* deleteChild(Tree* parent, Tree* child) {
	for (int i = 0; i < parent->children.size(); i++) {
		if (parent->children[i]->file->name == child->file->name) parent->children.erase(parent->children.begin() + i);
	}
	
	child->parent = NULL; //remove reference because it is no longer needed
	
	return child;
}



/* --------------------------------- *
 *  		FILE OPERATIONS			 *
 * --------------------------------- */
//merges blocks in the Ldisk
void mergeLD(list<Block> *LD) {
	list<Block>::iterator it = LD->begin();	
	list<Block>::iterator next;

	while (it != LD->end()) {	
		next = it;
		next++;
		
		if (next == LD->end()) return;
		
		if (it->free == next->free) {
			//set the endpoint of the block to the endpoint of the next block
			it->eid = next->eid;
			LD->erase(next);
		} else it++;
	}
}

//deallocation of file info
void deallocate(File *f, int block_size, list<Block> *LD) {
	for (int i = 0; i < (f->bytes - f->size)/block_size; i++) {
		list<int>::iterator it = f->addrs.end();//finds a block to erase
		it--;
		
		int test = *it;
		int erase_id = test / block_size; //finds the block ID
	
		Block* found;

		for (list<Block>::iterator it = LD->begin(); it != LD->end(); it++) {
			if (erase_id >= it->sid && erase_id <= it->eid) {
				found = &*it;
				break;
			}
		}
		
		//split the block
		Block deleted(erase_id, erase_id, true);
		//check the size of the block
		if (found->sid == found->eid) found->free = true;
		else if (found->eid - found->sid == 1) {	
			if (erase_id != found->sid) {
				found->eid = found->sid;
				
				if (found == &*(LD->end())) LD->push_back(deleted); // check if this is the last block stored in memory
				else {
					found++;
					LD->insert(LD->end(), deleted);
				}
			} else {
				found->sid = found->eid;
				LD->insert(LD->end(), deleted);
			}
		} else {
			if (found->eid == erase_id) { // check if we are at the last ID
				found->eid--;
				if (found == &*(LD->end())) {
					LD->push_back(deleted);
				} else {
					found++;
					LD->insert(LD->end(), deleted);
				}
			} else if (found->sid == erase_id) { //check if we are at the starting ID
				found->sid++;
				LD->insert(LD->end(), deleted);
			} else { //splitting the block
				Block before(found->sid, erase_id - 1, false);
				Block after(erase_id + 1, found->eid, false);
				
				LD->insert(LD->end(), before);				
				LD->insert(LD->end(), deleted);
				LD->insert(LD->end(), after);
				LD->erase(LD->end());			
			}
		}
				
		f->bytes -= block_size;
		f->addrs.erase(it);
	}

	mergeLD(LD);
}

//allocation of the file info
void allocate(File *file, int block_size, list<Block> *LD) {
	int diff = file->size - file->bytes;

	if (diff < 0) diff = 0;
	
	int blocks = ceil((diff * 1.0) / block_size);
	
	while (blocks > 0) { //loop until there are no more blocks
		int iter = 0;
		Block *found;

		for (list<Block>::iterator it = LD->begin(); it != LD->end(); it++) {
			if (it->free) {
				found = &*it;
				break;
			}
			iter++;
		}

		//If the disk is full
		if (iter == LD->size()) {
			file->size = file->bytes;
			mergeLD(LD);
			return;
		}
		
		// size of the block
		int found_size = found->eid-found->sid+1;
		
		if (blocks < found_size) {
			Block temp(found->sid, found->sid+blocks-1, false);
			for (int i = temp.sid; i <= temp.eid; i++) file->addrs.push_back(i * block_size);
			//add new blocks after splitting
			//update which blocks are now the start and end point
			LD->insert(LD->end(), temp);
			found->sid = temp.eid+1;
			file->bytes += blocks*block_size;
			blocks = 0;
		} else {
			found->free = false;
			file->bytes += found_size * block_size;
			blocks -= found_size;
			
			for (int i = found->sid; i <= found->eid; i++) file->addrs.push_back(i * block_size);
		}
	}

	mergeLD(LD);
}


/* --------------------------------- *
 *  	COMMAND LINE FUNCTIONS		 *
 * --------------------------------- */
//updates the current directories
void cli_cd(string dir, Tree *dir_current, Tree *dir_root) {
	if (dir == "..") { //cd to parent directory
		if (dir_current->parent == NULL) cout << "error: you are already at the root" << endl;
		else {
			*dir_current = (*dir_current->parent);
		}
		return;
	}

	Tree *found = find(dir_root, dir); // find the name of the directory
	if (found == NULL) cout << "error: cannot find directory" << endl; //no file was found
	else if (found->file->is_dir == false) cout << "error: invalid directory" << endl;
	else *dir_current = *found; //directory is found
}
//Prints the name of valid files
void cli_ls(Tree* dir_current) {
	for (int i = 0; i < dir_current->children.size(); i++) //
		cout << dir_current->children[i]->file->name << (dir_current->children[i]->file->is_dir ? "   (directory)" : "") << endl;
}

//creates a new directory and inserts it into our tree
void cli_mkdir(string newdir, Tree* dir_current) {
	File *f = new File(dir_current->file->name + "/" + newdir, 0, true);
	Tree *child = new Tree(f);

	dir_current->children.push_back(child);
	child->parent = dir_current;
}

//creatses a new file in our new directory and inserts it into our tree
void cli_create(string newfile, Tree* dir_current) {
	File *f = new File(dir_current->file->name + "/" + newfile, 0, false);
	Tree *child = new Tree(f);

	dir_current->children.push_back(child);
	child->parent = dir_current;
}
//if given a valid file, allocate the file and keep a record of its timestamp
//file size is then incremented
void cli_append(string name, string bytes, Tree* dir_root, int block_size, list<Block> *LD) {
	Tree *found = find(dir_root, name);
	
	if (found == NULL) cout << "error: could not find file" << endl;
	else if (found->file->is_dir) cout << "error: can not perform operation on directory" << endl;
	else {
		found->file->size += atoi(bytes.c_str());
		allocate(found->file, block_size, LD);
		time(&(found->file->timestamp));
	}
}

//If we are in a valid directory, file has its size decremented and is deallocated
void cli_remove(string name, string bytes, Tree* dir_root, int block_size, list<Block> *LD) {
	Tree *found = find(dir_root, name);
	
	if (found == NULL) {
		cout << "error: could not find file" << endl;
		return;
	} else if (found->file->is_dir) cout << "error: can not perform operation on directory" << endl;
	else { 
		found->file->size -= atoi(bytes.c_str());
		if (found->file->size < 0) found->file->size = 0;
		
		deallocate(found->file, block_size, LD);
		time(&(found->file->timestamp));
	}
}

//finds a node and deletes the directory if it is empty
void cli_delete(string name, Tree *dir_root, int block_size, list<Block> *LD) {
	Tree *found = find(dir_root, name);
	
	if (found == NULL) {
		cout << "error: could not find file" << endl;
		return;
	}

	Tree *parent = found->parent;
	
	if (!found->file->is_dir) {
		found->file->size = 0;
		deallocate(found->file, block_size, LD);
		time(&(parent->file->timestamp));
	} else if (found->children.size() != 0) cout << "error: non-empty directory cannot be removed" << endl;

	if ((!found->file->is_dir) || (found->file->is_dir && found->children.size() == 0)) {
		//delete child reference
		for (int i = 0; i < parent->children.size(); i++) {
			if (parent->children[i]->file->name == found->file->name) 
				parent->children.erase(parent->children.begin() + i);
		}
		found->parent = NULL;
	}
}

void cli_exit(Tree *dir_root) {
	vector<Tree*> trv = traverse(dir_root);

	for (vector<Tree*>::iterator it = trv.begin(); it != trv.end(); it++) {
		delete (*it)->file;
		delete (*it);
	}
}

//Adds the nodes of a tree to the children queue
//Will display the file info associated with each node
void cli_prfiles(Tree *dir_root) {
	vector<Tree*> trv = traverse(dir_root);

	for (vector<Tree*>::iterator it = trv.begin(); it != trv.end(); it++) {
		if (!(*it)->file->is_dir) (*it)->file->display(true);
	}
}

//Displays the free and in-use nodes
void cli_prdisk(Tree *dir_root, list<Block> *LD) {
	list<Block>::iterator it;
	
	for (it = LD->begin(); it != LD->end(); it++) {
		if (it->free) cout << "free: " << it->sid << "-" << it->eid << endl;
		else cout << "in use: " << it->sid << "-" << it->eid << endl;
	}
	
	int frag = 0;
	vector<Tree*> trv = traverse(dir_root);

	for (vector<Tree*>::iterator it = trv.begin(); it != trv.end(); it++) {
		if (!(*it)->file->is_dir) frag += (*it)->file->bytes - (*it)->file->size;
	}
	cout << "fragmentation: " << frag << " B" << endl;
}


/* --------------- *
 *  	MAIN       *
 * --------------- */
//reads the contents of a file
vector<string> read_in(string filename) {
	ifstream instream(filename);
	string instring;
	vector<string> out;

	while (getline(instream, instring)) {
		if (instring.size() == 0) continue;

		out.push_back(instring);
	}

	return out;
}

vector<string> split(string inp, char sep) {
	stringstream line(inp);
	string seg;
	vector<string> out;

	while (getline(line, seg, sep)) {
		if (seg.size() == 0 || seg.compare(" ") == 0) continue;
		out.push_back(seg);
	}

	return out;
}

int main(int argc, char **argv) {
	int block_size;
	int disk_size;
	char* dir_list_file;
	char* file_list_file;
	vector<string> dir_list;
	vector<string> file_list;

	Tree* dir_current;
	Tree* dir_root;

	list<Block> LD;

	bool o_fd = false;
	bool o_dd = false;
	bool o_bd = false;
	bool o_sd = false;

	/*Parsing command-line arguments*/
	for (int i = 1; i < argc-1; i += 2) {
		bool o_f = strcmp(argv[i], "-f") == 0;
		bool o_d = strcmp(argv[i], "-d") == 0;
		bool o_b = strcmp(argv[i], "-b") == 0;
		bool o_s = strcmp(argv[i], "-s") == 0;

		if (o_f) { file_list_file = argv[i+1]; o_fd = true; }
		else if (o_d) { dir_list_file = argv[i+1]; o_dd = true; }
		else if (o_b) { block_size = atoi(argv[i+1]); o_bd = true; }
		else if (o_s) { disk_size = atoi(argv[i+1]); o_sd = true; }
	}

	//Detect missing arguments and print them out
	if (!o_fd || !o_dd || !o_bd || !o_sd) {
		vector<string> missing;

		if (!o_fd) missing.push_back("-f");
		if (!o_dd) missing.push_back("-d");
		if (!o_bd) missing.push_back("-b");
		if (!o_sd) missing.push_back("-s");

		if (missing.size() > 0) {
			string str = "";
			for (vector<string>::iterator it = missing.begin(); it != missing.end(); it++) str += (*it) + ", ";
			cout << "missing arguments: " << str.substr(0, str.length()-2) << endl;
			return 1;
		}
	}

	try {
		dir_list = read_in(dir_list_file);
	} catch (int e) { //file not found in directory
		cout << "error: cannot read directory list, make sure there is a file with the specified name in this directory" << endl;
		return 1;
	}
	try {
		file_list = read_in(file_list_file);
	} catch (int e) { //could not read the contents of the file
		cout << "error: cannot read file list, make sure there is a file with the specified name in this directory" << endl;
		return 1;
	}
	// add an empty block to Ldisk
	Block block(0, ceil(disk_size * 1.0 / block_size) - 1, true);
	LD.push_back(block);

	string root_line = dir_list.at(0);
	dir_root = new Tree(new File(root_line.substr(0, root_line.find_last_of("/")), 0, true));

	for (vector<string>::iterator it = dir_list.begin()+1; it != dir_list.end(); it++) {
		File *file = new File(*it, 0, true);
		Tree *parent = find(dir_root, (*it).substr(0, (*it).find_last_of("/")));
		Tree *child = new Tree(file);
		parent->children.push_back(child);
		child->parent = parent;
	}


	for (vector<string>::iterator it = file_list.begin(); it != file_list.end(); it++) {
		vector<string> spl = split(*it, ' ');

		int file_size = atoi(spl[0].c_str());
		string file_name = spl[1];

		File *file = new File(file_name, file_size, false);
		Tree *parent = find(dir_root, file_name.substr(0, file_name.find_last_of("/")));
		Tree *child = new Tree(file);

		parent->children.push_back(child);
		child->parent = parent;

		allocate(file, block_size, &LD);
	}

	dir_current = dir_root;
	string userInput;
	vector<string> spl;


	while (true) {
		//display the current directory and handle user input
		cout << "Assignment 3:" << dir_current->file->name << "$ ";
		getline(cin, userInput);
		spl = split(userInput, ' ');
		string cmd = spl[0].c_str();

		if (cmd.compare("cd") == 0) {
			if (spl.size() != 2) {
				cout << "error: wrong number of parameters given" << endl;
				continue;
			}	
			cli_cd(spl[1], dir_current, dir_root);
		} else if (cmd.compare("ls") == 0) {
			if (spl.size() != 1) {
				cout << "error: wrong number of parameters given" << endl;
				continue;
			}	
			cli_ls(dir_current);
		} else if (cmd.compare("mkdir") == 0) {
			if (spl.size() != 2) {
				cout << "error: wrong number of parameters given" << endl;
				continue;
			}	
			cli_mkdir(spl[1], dir_current);
		} else if (cmd.compare("create") == 0) {
			if (spl.size() != 2) {
				cout << "error: wrong number of parameters given" << endl;
				continue;
			}	
			cli_create(spl[1], dir_current);
		} else if (cmd.compare("append") == 0) {
			if (spl.size() != 3) {
				cout << "error: wrong number of parameters given" << endl;
				continue;
			}	
			cli_append(spl[1], spl[2], dir_root, block_size, &LD);
		} else if (cmd.compare("remove") == 0) {
			if (spl.size() != 3) {
				cout << "error: wrong number of parameters given" << endl;
				continue;
			}	
			cli_remove(spl[1], spl[2], dir_root, block_size, &LD);
		} else if (cmd.compare("delete") == 0) {
			if (spl.size() != 2) {
				cout << "error: wrong number of parameters given" << endl;
				continue;
			}	
			cli_delete(spl[1], dir_root, block_size, &LD);
		} else if (cmd.compare("exit") == 0) {
			if (spl.size() != 1) {
				cout << "error: wrong number of parameters given" << endl;
				continue;
			}	
			cli_exit(dir_root);
		} else if (cmd.compare("dir") == 0) {
			if (spl.size() != 1) {
				cout << "error: wrong number of parameters given" << endl;
				continue;
			}	
			tree_bfs(dir_root);
		} else if (cmd.compare("prfiles") == 0) {
			if (spl.size() != 1) {
				cout << "error: wrong number of parameters given" << endl;
				continue;
			}	
			cli_prfiles(dir_root);
		} else if (cmd.compare("prdisk") == 0) {
			if (spl.size() != 1) {
				cout << "error: wrong number of parameters given" << endl;
				continue;
			}	
			cli_prdisk(dir_root, &LD);
		} else cout << "Invalid command entered" << endl;
	}
	
	return 0;
}


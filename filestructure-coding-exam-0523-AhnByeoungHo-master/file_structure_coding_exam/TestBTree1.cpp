//tstbtree.cc
#include "BTNode.h"
#include "recfile.h"
#include "recordng.h"
#include "BTree.h"
#include <iostream>
#include <string>
#define NumberOfRecords 10
using namespace std;

const int BTreeSize = 5;//btree order m = 5

template <class keyType>;
int RetrieveRecording(Recording & recording, string key, Btree & RecordingIndex, RecordFile<Recording> & RecordingFile)
{
	int Result;
	Result = RecordingFile.Read(RecordingIndex.Search(key));
	if (Result == -1) return FALSE;
	Result = recording.Unpack(RecordingFile.GetBuffer());
	return Result;
}

int main(int argc, char * argv)
{
	int result, i;
	int choice;
	string searchKey;
	Recording record;
	FixedFieldBuffer Buffer(300, 100);
	RecordFile<Recording> Recfile(Buffer);
	Recfile.Create("recording.dat", ios::out);
	BTree <string> bt(BTreeSize);
	result = bt.Create("btindex.dat", ios::in | ios::out);
	if (!result) { cout << "Creating of BTree index fails" << endl; return 0; }
	do
	{
		cout << "1.Insert all records"<<endl;//�����ؾ� ��
		cout << "2.Delete a record"<<endl;//�����ؾ� ��
		cout << "3.Search with a key"<<endl;//�����ؾ� ��
		cout << "4.Display all record"<<endl;//in-order traversal //�����ؾ� ��
		cout << "5.Exit"<<endl;
		cout << "Enter your choice : ";
		cin >> choice;
		switch (choice)
		{
		case 1:
			int recaddr;
			Recording * R[NumberOfRecords];
			R[0] = new Recording("LON", "2312", "Romeo and Juliet", "Prokofiev", "Maazel");
			R[1] = new Recording("RCA", "2626", "Quartet in C Sharp Minor", "Beethoven", "Julliard");
			R[2] = new Recording("WAR", "23699", "Touchstone", "Corea", "Corea");
			R[3] = new Recording("ANG", "3795", "Symphony No. 9", "Beethoven", "Giulini");
			R[4] = new Recording("COL", "38358", "Nebraska", "Springsteen", "Springsteen");
			R[5] = new Recording("DG", "18807", "Symphony No. 9", "Beethoven", "Karajan");
			R[6] = new Recording("MER", "75016", "Coq d'or Suite", "Rimsky-Korsakov", "Leinsdorf");
			R[7] = new Recording("COL", "31809", "Symphony No. 9", "Dvorak", "Bernstein");
			R[8] = new Recording("DG", "139201", "Violin Concerto", "Beethoven", "Ferras");
			R[9] = new Recording("FF", "245", "Good News", "Sweet Honey in the Rock", "Sweet Honey in the Rock");
			for (int i = 0; i<NumberOfRecords; i++)
			{
				R[i]->Pack(Buffer);
				recaddr = Recfile.Write(*R[i]);
				cout << "Recordin2 R[" << i << "] at recaddr " << recaddr << endl;
				result = bt.Insert(R[i]->IdNum, recaddr);
			}
		
			break;
		case 2:
			cout << bt.Remove("COL") << endl;
			break;
		case 3:
			cout << "Search key = "<<endl;
			cin >> searchKey;
			RetrieveRecording(record, searchKey, bt, Recfile);
			cout << record << endl;//operator <<�� �����ؾ� �Ѵ�
			break;
		case 4:
			cout << bt;//operator <<�� friend function���� �����Ѵ�
			//bt�� ��� key value�� ����ؾ� �Ѵ�.
			break;
		default:
			cout << "Wrong choice\n";
		}
	} while (choice != 4);
	return 1;
}
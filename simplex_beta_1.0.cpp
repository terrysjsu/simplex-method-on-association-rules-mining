/* simplex_beta_1.0.cpp
 *
 * Rui Tong
 * Department of Computer Science
 * San Jose State University
 *
 * Program Output: do it later
 *
 */ 
#include<stdio.h>
#include<stdlib.h>
#include<string>
#include<time.h>
#include<list>
#include<sstream>
#include<iostream>
#include <fstream>
using namespace std;
/***** Data Structure *****/
/* Description:
 * structure of table: 
 * bool ** table: 2-d table array representing real database
 * int * index: item name
 * int * support: total support of each item
 * int num_row: table's row
 * int num_column: table's column
 * string str_item: the frequent pattern for current table
 * int count: count for the current str_item
 */
typedef struct Table *Table_pointer;	/* Pointer to a FP-tree node */

typedef struct Table {
	bool ** table; //point to a table
	int * index; //the item name array
	int * support; // vertical total number of each item
	int num_row;
	int num_column;
	string str_item; // the parent string for this table
	int count; //the count for the item of current table
} Table;

/***** Global Variables *****/
int expectedK;			/* User input upper limit of itemset size to be mined */
int realK;			/* Actual upper limit of itemset size can be mined */
int threshold;			/* User input support threshold */
int numItem;			/* Number of items in the database */
int numTrans;			/* Number of transactions in the database */
char dataFile[100];		/* File name of the database */
char outFile[100];		/* File name to store the result of mining */

list<Table> table_list;         /* List of table and sub-table*/
/*char form for each item*/
string abcd = "~ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890!@#$%^&*()-+=[]{}";


/******************************************************************************************
 * Function: init_table()
 *
 * Description:
 *	Scan the DB and find the support of each item.
 *	form the original table
 */
void init_table(Table_pointer tp)
{
 int transSize;
 int item;
 int maxSize=0;
 FILE *fp;
 int i, j;

 tp->table = new bool*[numTrans];
 for(i=0;i<numTrans;i++)
 {
	 tp->table[i] = new bool[numItem];
	 memset(tp->table[i], 0, numItem);
 }

 tp->support = new int[numItem];
 memset(tp->support, 0, numItem*4);//memset function, the third parameter is based on unsigned char

 tp->index = new int[numItem];
 for(i=0;i<numItem;i++)
	 tp->index[i] = i;

 tp->num_row = numTrans;
 tp->num_column = numItem;
 tp->count = 0;
 tp->str_item = "";
 /* scan DB to count the frequency of each item */

 if ((fp = fopen(dataFile, "r")) == NULL) {
        printf("Can't open data file, %s.\n", dataFile);
        exit(1);
 }

 /* Scan each transaction of the DB */
 for (i=0; i < numTrans; i++) {

	/* Read the transaction size */
	fscanf(fp, "%d", &transSize);

	/* Mark down the largest transaction size found so far */
	if (transSize > maxSize)
		maxSize = transSize;

	/* Read the items in the transaction */
	for (j=0; j < transSize; j++) {
		fscanf(fp, "%d", &item);
		tp->support[item]++;
		tp->table[i][item]=1;
	}
 } 
 fclose(fp);

 return;
}

/******************************************************************************************
 * Function: input()
 *
 * Description:
 *	Read the input parameters from the configuration file.
 */
void input(char *configFile)
{
 FILE *fp;
 float thresholdDecimal;

 if ((fp = fopen(configFile, "r")) == NULL) {
        printf("Can't open config. file, %s.\n", configFile);
        exit(1);
 }

 fscanf(fp, "%d %f %d %d", &expectedK, &thresholdDecimal, &numItem, &numTrans);
 fscanf(fp, "%s %s", dataFile, outFile);
 fclose(fp);

 printf("expectedK = %d\n", expectedK);
 printf("thresholdDecimal = %f\n", thresholdDecimal);
 printf("numItem = %d\n", numItem);
 printf("numTrans = %d\n", numTrans);
 printf("dataFile = %s\n", dataFile);
 printf("outFile = %s\n\n", outFile);
 threshold = thresholdDecimal * numTrans;
 if (threshold == 0) threshold = 1;
 printf("threshold = %d\n", threshold);

 return;
}
/******************************************************************************************
 *Function: show_time()
 * Description: show the time 
 */
void show_time(int i){
	float time=(float)clock()/CLOCKS_PER_SEC;
	printf("time %d: %.4f secs.\n", i, time);
}

/******************************************************************************************
 *Function: destroy()
 * Description: destroy the table, free the memory
 */
void destroy(Table_pointer tp)
{
	for( int i = 0 ; i < tp->num_row ; i++ )
	delete [] tp->table[i];
	delete [] tp->table;

	delete [] tp->index;
	delete [] tp->support;

	delete tp;
}
/******************************************************************************************
 *Function: shrink_table()
 *
 *Description: only horizontal shrink table
 * copy and delete the previous one
 */
Table_pointer shrink_table(Table_pointer tp)
{
	Table_pointer p = new Table;
	int ncolumn = 0;//new num_column
	for(int i=0;i<tp->num_column;i++)
	{
		if(tp->support[i]>=threshold)
			ncolumn++;
	}
	p->num_column = ncolumn;
	p->num_row = tp->num_row;
	p->str_item = tp->str_item;
	p->count = tp->count;

	p->index = new int [p->num_column];//new index array
	p->support = new int [p->num_column];//new support array

	p->table = new bool*[p->num_row];
	if(p->table == 0){
		cout<<"out of memory"<<endl;
		exit(0);
	}
	for(int i=0;i<p->num_row;i++){
		p->table[i] = new bool[p->num_column];
		if(p->table[i] == 0){
		cout<<"out of memory"<<endl;
		exit(0);
		}
	}

	int taken_row = 0;//used to iterate new array
	for(int i=0;i<tp->num_column;i++)//copy into new 2-d array p[][]
	{
		if(tp->support[i]>=threshold)
		{
			p->support[taken_row] = tp->support[i];//copy new support
			p->index[taken_row] = tp->index[i];//copy new index
			for(int j= 0;j<tp->num_row;j++)
				p->table[j][taken_row] = tp->table[j][i];//copy 0 or 1
			taken_row++;
		}
	}

	/*delete previous table*/
	destroy(tp);
	
	return p;
}

/******************************************************************************************
 *Function: init_list
 *
 *Description: break the previous table into sub-table and insert into list<table>.
 * loop through the column-1 item, on each item prune the row which the item column is not '1'
 * for example:
 * a b c d
 * 1 0 1 0
 * 0 1 1 1
 * 1 1 0 1
 * 1 0 1 1
 * will results these table:
 * a-sub-table		b-sub-table	c-sub-table
 * b c d		c d		d
 * 0 1 0		1 1		0
 * 1 0 1		0 1		1
 * 0 1 1				1
 */
void init_list(Table_pointer tp)
{
	for(int i=0;i<tp->num_column-1;i++)//i loop through each item in table
	{
		char item = abcd[tp->index[i]];//item name in char

		Table_pointer listp = new Table;//new sub-table: support[i] X (num_column-1 - i)

		/*initial value*/
		listp->num_column = 0;
		listp->num_row = 0;
		listp->count = 0;
		listp->index = 0;
		listp->str_item = "";
		listp->support = 0;
		listp->table = 0;

		/*column and row*/
		listp->num_column = tp->num_column-i-1;//sub table column
		listp->num_row = tp->support[i];

		listp->table = new bool*[listp->num_row];//support num is the row num of new sub table
		for(int k=0;k<listp->num_row;k++)
			listp->table[k] = new bool[listp->num_column];

		/*index, support, string and count*/
		listp->index = new int[listp->num_column];
		listp->support = new int[listp->num_column];
		memset(listp->support,0,listp->num_column*sizeof(int));

		/*sub table*/
		int m =0;
		for(int j=0;j<tp->num_row;j++)//j loop through row to find 1
		{
			if(tp->table[j][i] == 1){
				//memcpy(listp->table[m++], &(tp->table[j][i+1]), sizeof(bool)*listp->num_column);
				//copy each row into new array, however, don't use memcpy() because have to calculate support here:
				for(int g=0;g<listp->num_column;g++)
				{
					bool tmp = tp->table[j][g+i+1];
					listp->table[m][g] = tmp;
					if(tmp == 1)
						listp->support[g]++;
				}
				m++;
			}
		}

		/*index, string, count*/
		for(int n=0;n<listp->num_column;n++)
		{
			listp->index[n] = tp->index[i+n+1];
		}
		listp->str_item = tp->str_item + item;
		listp->count = tp->support[i];

		/*shrink by threshold*/
		Table_pointer newp = shrink_table(listp);//very important in this algorithm!

		if(newp->num_column >0)
			table_list.push_back(*newp);
	}
	destroy(tp);
}
/******************************************************************************************
 * Function: traverse_list()
 *
 * Description: traverse the list, output current table's association rules, and push back the table which is not null
 *
 */
void traverse_list()
{
	list<Table>::iterator lt = table_list.begin();
	while(lt != table_list.end())
	{
		/*output*/
		for(int i=0;i<lt->num_column;i++){
			if(lt->support[i] >= threshold)
			cout<<"  "<<lt->str_item+abcd[lt->index[i]]<<" ["<<((lt->count<lt->support[i])?lt->count:lt->support[i])<<"]"<<endl;
		}

		/*sub-sub table*/
		if(lt->num_column >1)
		for(int i=0;i<lt->num_column-1;i++)
		{
			char item = abcd[lt->index[i]];

			Table_pointer np = new Table;//new 
			
			np->num_column = lt->num_column-i-1;//sub table column
			np->num_row = lt->support[i];
	
			np->table = new bool*[np->num_row];//support num is the row num of new sub table
			for(int k=0;k<np->num_row;k++)
				np->table[k] = new bool[np->num_column];
	
			/*index, support, string and count*/
			np->index = new int[np->num_column];
			np->support = new int[np->num_column];
			memset(np->support,0,np->num_column*sizeof(int));
	
			/*sub table*/
			int m =0;
			for(int j=0;j<lt->num_row;j++)//j loop through row to find 1
			{
				if(lt->table[j][i] == 1){
					//memcpy(listp->table[m++], &(tp->table[j][i+1]), sizeof(bool)*listp->num_column);
					//copy each row into new array, however, don't use memcpy() because have to calculate support here:
					for(int g=0;g<np->num_column;g++)
					{
						bool tmp = lt->table[j][g+i+1];
						np->table[m][g] = tmp;
						if(tmp == 1)
							np->support[g]++;
					}
					m++;
				}
			}
	
			/*index, string, count*/
			for(int n=0;n<np->num_column;n++)
			{
				np->index[n] = lt->index[i+n+1];
			}
			np->str_item = lt->str_item + item;
			np->count = lt->support[i];

		/*shrink by threshold*/
			Table_pointer lp = shrink_table(np);

			if(lp->table)
				table_list.push_back(*lp);// add the table into the list for next processing
		}

		/*destroy previous table*/
		for( int i = 0 ; i < lt->num_row ; i++ )
			delete [] lt->table[i];
		delete [] lt->table;

		delete [] lt->index;
		delete [] lt->support;
		
		lt++;
	}
}
/******************************************************************************************
 * Function: main
 */
void main(int argc, char *argv[])
{
 if (argc != 2) {
        printf("Usage: %s <config. file>\n\n", argv[0]);
        exit(1);
 }
 /* read input parameters --------------------------*/
 printf("input\n");
 input(argv[1]);

 Table_pointer op = new Table;
 show_time(1);

 init_table(op);
 Table_pointer tp =  shrink_table(op);

 init_list(tp);

 traverse_list();
 
 show_time(2);
 
 return;
}


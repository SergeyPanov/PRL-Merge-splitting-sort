/*
 * algorithm: odd-even transposition sort (alg. ~40 lines long)
 * author: jakub zak
 *
 */

#include "/Users/sergeypanov/bin/mpi/include/mpi.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>


using namespace std;
#define TAG 0

void send(vector<int> const& vec, int dest){
    int len = static_cast<int>(vec.size());
    MPI_Request  request;
    MPI_Isend(&len, 1, MPI_UNSIGNED, dest, TAG, MPI_COMM_WORLD, &request);

    if (len != 0)
        MPI_Isend(vec.data(), len, MPI_INT, dest, TAG, MPI_COMM_WORLD, &request);
}


void recv(vector<int>& vec, int src){
    unsigned len;
    MPI_Status stat;
    MPI_Recv(&len, 1, MPI_UNSIGNED, src, TAG, MPI_COMM_WORLD, &stat); //Vezme pocet cisel
    if (len != 0){
        vec.resize(len);
        MPI_Recv(vec.data(), len, MPI_INT, src, TAG, MPI_COMM_WORLD, &stat); //Nacte je do vektoru
    } else{
        vec.clear();
    }
}

void showVector(vector<int>& vec, int me){
    for (int i = 0; i < vec.size(); ++i) {
        cout << "id: " << me << ' ' << vec[i] << endl;
    }
}

int main(int argc, char *argv[])
{
    int numprocs;               //pocet procesoru
    int myid;                   //muj rank
    int neighnumber;            //hodnota souseda
    vector<int> neigh_ints;
    int mynumber;               //moje hodnota
    vector<int> my_ints;        //Vektor mych hodnot
    MPI_Status stat;            //struct- obsahuje kod- source, tag, error
    int numbs_for_proc = 0;         //Pocet cisel pro kazdy procesor

    //MPI INIT
    MPI_Init(&argc,&argv);                          // inicializace MPI 
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);       // zjistÃ­me, kolik procesÅ¯ bÄ›Å¾Ã­ 
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);           // zjistÃ­me id svÃ©ho procesu 

    //NACTENI SOUBORU
    /* -proc s rankem 0 nacita vsechny hodnoty
     * -postupne rozesle jednotlive hodnoty vsem i sobe
    */
    vector<int> unsortedInts;
    if(myid == 0){
        char input[]= "numbers";                          //Jmeno souboru
        int number;                                     //Hodnota pri nacitani souboru
        int invar= 0;                                   //Invariant- urcuje cislo proc, kteremu se bude posilat
        fstream fin;                                    //Cteni ze souboru
        fin.open(input, ios::in);

        while(fin.good()){
            number= fin.get();
            if(!fin.good()) break;                      //Nacte i eof, takze vyskocim
            unsortedInts.push_back(number);             //Nacte vse cisla ze vstupniho souboru
        }//while
        fin.close();

        for (int j = 0; j < unsortedInts.size(); ++j) {
            if (j == unsortedInts.size() - 1)
                cout << unsortedInts[j] << endl;
            else
                cout << unsortedInts[j] << ' ';
        }


        // Rozesle vsem procesorem posloupnosti cisel
        int index = 0;
        invar = 0;
        numbs_for_proc = static_cast<int>(ceil(unsortedInts.size() / (double)numprocs));
        for (int proc_id = 0; proc_id < numprocs; ++proc_id) {
            vector<int> message;


            for (int i = index; i < index + numbs_for_proc; ++i) {
                if (i >= unsortedInts.size()) break;
                message.push_back(unsortedInts[i]);
            }

            index += numbs_for_proc;
            send(message, proc_id);

            ++invar;
        }




    }//nacteni souboru

    //PRIJETI HODNOTY CISLA
    //vsechny procesory(vcetne mastera) prijmou hodnotu a zahlasi ji

    recv(my_ints, 0);

//    cout << "Initial " << endl;
//    for (int i = 0; i < my_ints.size(); ++i) {
//        cout << "I'm " << myid << " got number " << my_ints[i] << endl;
//    }

    sort(my_ints.begin(), my_ints.end());   // Seradi optimalnim linearnim algoritmem


    //LIMIT PRO INDEXY
    int oddlimit= 2*(numprocs/2)-1;                 //limity pro sude
    int evenlimit= 2*((numprocs-1)/2);              //liche
    int halfcycles= ceil(numprocs/(double)2);
    int cycles=0;                                   //pocet cyklu pro pocitani slozitosti
    //if(myid == 0) cout<<oddlimit<<":"<<evenlimit<<endl;

    //RAZENI------------chtelo by to umet pocitat cykly nebo neco na testy------
    //cyklus pro linearitu
    for(int j=1; j<=halfcycles; j++){
        cycles++;           //pocitame cykly, abysme mohli udelat krasnej graf:)


        //sude proc 
        if((!(myid%2) || myid==0) && (myid<oddlimit)){
            send(my_ints, myid+1);

            recv(my_ints, myid + 1);
        }//if sude
        else if(myid<=oddlimit){//liche prijimaji zpravu a vraceji mensi hodnotu (to je ten swap)

            recv(neigh_ints, myid - 1);

            //Spojit vektory
            vector<int> merged_ints;
            merged_ints.reserve(my_ints.size() + neigh_ints.size());
            merged_ints.insert(merged_ints.end(), my_ints.begin(), my_ints.end());
            merged_ints.insert(merged_ints.end(), neigh_ints.begin(), neigh_ints.end());
            sort(merged_ints.begin(), merged_ints.end());


            neigh_ints.clear();
            for (int i = 0; i < merged_ints.size() / 2; ++i) {
                neigh_ints.push_back(merged_ints[i]);
            }
            my_ints.clear();
            for (int i = static_cast<int>(merged_ints.size() / 2); i < merged_ints.size(); ++i) {
                my_ints.push_back(merged_ints[i]);
            }

            send(neigh_ints, myid - 1);
        }//else if (liche)
        else{//sem muze vlezt jen proc, co je na konci
        }//else

        //liche proc 
        if((myid%2) && (myid<evenlimit)){

            send(my_ints, myid+1);

            recv(my_ints, myid + 1);


        }//if liche
        else if(myid<=evenlimit && myid!=0){//sude prijimaji zpravu a vraceji mensi hodnotu (to je ten swap)

            recv(neigh_ints, myid - 1);

            //Spojit vektory
            vector<int> merged_ints;
            merged_ints.reserve(my_ints.size() + neigh_ints.size());
            merged_ints.insert(merged_ints.end(), my_ints.begin(), my_ints.end());
            merged_ints.insert(merged_ints.end(), neigh_ints.begin(), neigh_ints.end());
            sort(merged_ints.begin(), merged_ints.end());


            my_ints.clear();
            for (int i = static_cast<int>(merged_ints.size() / 2); i < merged_ints.size(); ++i) {
                my_ints.push_back(merged_ints[i]);
            }

            neigh_ints.clear();
            for (int i = 0; i < merged_ints.size() / 2; ++i) {
                neigh_ints.push_back(merged_ints[i]);
            }

            send(neigh_ints, myid - 1);

        }//else if (sude)
        else{//sem muze vlezt jen proc, co je na konci
        }//else

    }//for pro linearitu
    //RAZENI--------------------------------------------------------------------

    //FINALNI DISTRIBUCE VYSLEDKU K MASTEROVI-----------------------------------
    int* final= new int [numprocs];
    vector<int> sorted;
    //final=(int*) malloc(numprocs*sizeof(int));
    for(int i=1; i<numprocs; i++){
        if(myid == i){
//            cout << "I'm: " << myid<< endl;
//            showVector(my_ints, myid);
            send(my_ints, 0);
        }
        if(myid == 0){
            recv(neigh_ints, i);
            sorted.insert(sorted.end(), neigh_ints.begin(), neigh_ints.end());

        }//if sem master
    }//for

    if(myid == 0){
        sorted.insert(sorted.begin(), my_ints.begin(), my_ints.end());
        for (int i = 0; i < sorted.size(); ++i) {
            cout << sorted[i] << endl;
        }
    }//if vypis
    //cout<<"i am:"<<myid<<" my number is:"<<mynumber<<endl;
    //VYSLEDKY------------------------------------------------------------------


    MPI_Finalize();





    return 0;

}//main

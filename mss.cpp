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

using namespace std;

#define TAG 0

void send(vector<int> const& vec, int dest){
    int len = static_cast<int>(vec.size());
    MPI_Send(&len, 1, MPI_UNSIGNED, dest, TAG, MPI_COMM_WORLD);
    if (len != 0)
        MPI_Send(vec.data(), len, MPI_INT, dest, TAG, MPI_COMM_WORLD);
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
        char input[]= "numbers";                          //jmeno souboru    
        int number;                                     //hodnota pri nacitani souboru
        int invar= 0;                                   //invariant- urcuje cislo proc, kteremu se bude posilat
        fstream fin;                                    //cteni ze souboru
        fin.open(input, ios::in);

        while(fin.good()){
            number= fin.get();
            if(!fin.good()) break;                      //nacte i eof, takze vyskocim
            unsortedInts.push_back(number);             //Nacte vse cisla ze vstupniho souboru
//            MPI_Send(&number, 1, MPI_INT, invar, TAG, MPI_COMM_WORLD); //buffer,velikost,typ,rank prijemce,tag,komunikacni skupina
            invar++;
        }//while
        fin.close();


        // Rozesle vsem procesorem posloupnosti cisel
        int index = 0;
        invar = 0;
        numbs_for_proc = static_cast<int>(ceil(unsortedInts.size() / (double)numprocs));

        for (int proc_id = 0; proc_id < numprocs; ++proc_id) {
            vector<int> message;

            for (int i = index; i < index + numbs_for_proc; ++i) {
                if (i >= unsortedInts.size()){
                    message.push_back(0);
                } else{
                    message.push_back(unsortedInts[i]);
                }
            }
            index += numbs_for_proc;
            send(message, invar);
            ++invar;
        }

    }//nacteni souboru

    //PRIJETI HODNOTY CISLA
    //vsechny procesory(vcetne mastera) prijmou hodnotu a zahlasi ji


    recv(my_ints, 0);

    sort(my_ints.begin(), my_ints.end());   // Seradi optimalnim linearnim algoritmem
    cout << "Initial " << endl;
    for (int i = 0; i < my_ints.size(); ++i) {
        cout << "I'm " << myid << " got number " << my_ints[i] << endl;
    }


    //LIMIT PRO INDEXY
    int oddlimit= 2*(numprocs/2)-1;                 //limity pro sude
    int evenlimit= 2*((numprocs-1)/2);              //liche
    int halfcycles= numprocs/2;
    int cycles=0;                                   //pocet cyklu pro pocitani slozitosti
    //if(myid == 0) cout<<oddlimit<<":"<<evenlimit<<endl;


    //RAZENI------------chtelo by to umet pocitat cykly nebo neco na testy------
    //cyklus pro linearitu
    for(int j=1; j<=halfcycles; j++){
        cycles++;           //pocitame cykly, abysme mohli udelat krasnej graf:)

        //sude proc 
        if((!(myid%2) || myid==0) && (myid<oddlimit)){
//            showVector(my_ints, myid);
//            cout << "------" << endl;
//            MPI_Send(&mynumber, 1, MPI_INT, myid+1, TAG, MPI_COMM_WORLD);          //poslu sousedovi svoje cislo
            send(my_ints, myid+1);
//            MPI_Recv(&mynumber, 1, MPI_INT, myid+1, TAG, MPI_COMM_WORLD, &stat);   //a cekam na nizsi
            recv(my_ints, myid + 1);
//            cout<<"ss: "<<myid<<endl;
//            cout << "Received numbers" << endl;
//            for (int i = 0; i < my_ints.size(); ++i) {
//                cout << "I'm " << myid << " got number " << my_ints[i] << endl;
//            }

        }//if sude
        else if(myid<=oddlimit){//liche prijimaji zpravu a vraceji mensi hodnotu (to je ten swap)
//            MPI_Recv(&neighnumber, 1, MPI_INT, myid-1, TAG, MPI_COMM_WORLD, &stat); //jsem sudy a prijimam

            recv(neigh_ints, myid - 1);

            //Spojit vektory
            vector<int> merged_ints;
            merged_ints.reserve(my_ints.size() + neigh_ints.size());
            merged_ints.insert(merged_ints.end(), my_ints.begin(), my_ints.end());
            merged_ints.insert(merged_ints.end(), neigh_ints.begin(), neigh_ints.end());
            sort(merged_ints.begin(), merged_ints.end());


            cout << "Merged: " << myid << endl;
            showVector(merged_ints, myid);

            neigh_ints.clear();
            for (int i = 0; i < merged_ints.size() / 2; ++i) {
                neigh_ints.push_back(merged_ints[i]);
            }

            cout << "Neight of: " << myid << endl;
            showVector(neigh_ints, myid);

            my_ints.clear();
            for (int i = static_cast<int>(merged_ints.size() / 2); i < merged_ints.size(); ++i) {
                my_ints.push_back(merged_ints[i]);
            }
            cout << "My: " << myid << endl;
            showVector(my_ints, myid);



            send(neigh_ints, myid - 1);
//
//            if(neighnumber > mynumber){                                             //pokud je leveho sous cislo vetsi
//                MPI_Send(&mynumber, 1, MPI_INT, myid-1, TAG, MPI_COMM_WORLD);       //poslu svoje
//                mynumber= neighnumber;                                              //a vemu si jeho
//            }
//            else MPI_Send(&neighnumber, 1, MPI_INT, myid-1, TAG, MPI_COMM_WORLD);   //pokud je mensi nebo stejne vratim
            //cout<<"sl: "<<myid<<endl;
        }//else if (liche)
        else{//sem muze vlezt jen proc, co je na konci
        }//else

        //liche proc 
        if((myid%2) && (myid<evenlimit)){
//            showVector(my_ints, myid);
//            cout << "------" << endl;
//            MPI_Send(&mynumber, 1, MPI_INT, myid+1, TAG, MPI_COMM_WORLD);           //poslu sousedovi svoje cislo
            send(my_ints, myid+1);
//            MPI_Recv(&mynumber, 1, MPI_INT, myid+1, TAG, MPI_COMM_WORLD, &stat);    //a cekam na nizsi
            recv(my_ints, myid + 1);
//            cout<<"ll: "<<myid<<endl;
//            cout << "Received numbers" << endl;
//            for (int i = 0; i < my_ints.size(); ++i) {
//                cout << "I'm " << myid << " got number " << my_ints[i] << endl;
//            }

        }//if liche
        else if(myid<=evenlimit && myid!=0){//sude prijimaji zpravu a vraceji mensi hodnotu (to je ten swap)
//            MPI_Recv(&neighnumber, 1, MPI_INT, myid-1, TAG, MPI_COMM_WORLD, &stat); //jsem sudy a prijimam
            recv(neigh_ints, myid - 1);

            //Spojit vektory
            vector<int> merged_ints;
            merged_ints.reserve(my_ints.size() + neigh_ints.size());
            merged_ints.insert(merged_ints.end(), my_ints.begin(), my_ints.end());
            merged_ints.insert(merged_ints.end(), neigh_ints.begin(), neigh_ints.end());
            sort(merged_ints.begin(), merged_ints.end());

            cout << "Merged: " << myid << endl;
            showVector(merged_ints, myid);
            
            neigh_ints.clear();
            for (int i = static_cast<int>(merged_ints.size() / 2); i < merged_ints.size(); ++i) {
                neigh_ints.push_back(merged_ints[i]);
            }

            cout << "Neight of: " << myid << endl;
            showVector(neigh_ints, myid);

            my_ints.clear();
            for (int i = 0; i < merged_ints.size() / 2; ++i) {
                my_ints.push_back(merged_ints[i]);
            }
            cout << "My: " << myid << endl;
            showVector(my_ints, myid);


            send(neigh_ints, myid - 1);

//            if(neighnumber > mynumber){                                             //pokud je leveho sous cislo vetsi
//                MPI_Send(&mynumber, 1, MPI_INT, myid-1, TAG, MPI_COMM_WORLD);       //poslu svoje
//                mynumber= neighnumber;                                              //a vemu si jeho
//            }
//            else MPI_Send(&neighnumber, 1, MPI_INT, myid-1, TAG, MPI_COMM_WORLD);   //pokud je mensi nebo stejne vratim
            //cout<<"ls: "<<myid<<endl;
        }//else if (sude)
        else{//sem muze vlezt jen proc, co je na konci
        }//else
        cout << "------------------------" << endl;

    }//for pro linearitu
    //RAZENI--------------------------------------------------------------------

    //FINALNI DISTRIBUCE VYSLEDKU K MASTEROVI-----------------------------------
    int* final= new int [numprocs];
    vector<int> sorted;
    //final=(int*) malloc(numprocs*sizeof(int));
    for(int i=1; i<numprocs; i++){
        if(myid == i){
//            for (int j = 0; j < my_ints.size(); ++j) {
//                cout << "MY id: " << myid << " " << my_ints[j] << endl;
//            }
            send(my_ints, 0);
//            MPI_Send(&mynumber, 1, MPI_INT, 0, TAG,  MPI_COMM_WORLD);
        }
        if(myid == 0){
            recv(neigh_ints, i);
            sorted.insert(sorted.end(), neigh_ints.begin(), neigh_ints.end());
//            MPI_Recv(&neighnumber, 1, MPI_INT, i, TAG, MPI_COMM_WORLD, &stat); //jsem 0 a prijimam
//            final[i]=neighnumber;
        }//if sem master
    }//for

    if(myid == 0){
//        for (int j = 0; j < my_ints.size(); ++j) {
//            cout << "MY id: " << myid << " " << my_ints[j] << endl;
//        }
        //cout<<cycles<<endl;
//        final[0]= mynumber;
        sorted.insert(sorted.begin(), my_ints.begin(), my_ints.end());
        for (int i = 0; i < sorted.size(); ++i) {
            cout << sorted[i] << endl;
        }
//        for(int i=0; i<numprocs; i++){
//            cout<<"proc: "<<i<<" num: "<<final[i]<<endl;
//        }//for
    }//if vypis
    //cout<<"i am:"<<myid<<" my number is:"<<mynumber<<endl;
    //VYSLEDKY------------------------------------------------------------------


    MPI_Finalize();
    return 0;

}//main

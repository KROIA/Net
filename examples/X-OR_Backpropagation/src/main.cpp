#include <QCoreApplication>
#include <backpropnet.h>
#include <iostream>
#include <windows.h>

#include <QString>
#include <QDebug>
#include<stdio.h>
#include<time.h>

#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <random>
#include <cmath>


using namespace std;

static std::vector<std::vector<double>    >trainingsSet;  //Trainings input std::vector
static std::vector<std::vector<double>    >outputSet;     //Trainings expected output std::vector


static FILE *genomlogFile;
static FILE *logfile;

static std::chrono::high_resolution_clock::time_point t2;
static std::chrono::duration<double> time_span;
static std::chrono::high_resolution_clock::time_point t1;

void printNet(Net *net);
void setupTrainingSet();
void netFinished(BackpropNet *net);
void cmdXY(unsigned int x,unsigned int y);
void logGenom(std::vector<double> genom);
class ErrorHandler;

static std::vector<std::vector<std::vector<double> >   > _genomCompareList;
static std::vector<std::vector<double>  >_runtimeList;
static unsigned int reserveSize = 10;


//static ErrorHandler    *errorHandler;
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    setupTrainingSet(); //Setting the trainingset for the X-OR problem

    unsigned int netID              = 0;
    unsigned int inputNeurons       = trainingsSet[0].size(); //Makes the amount of inputs dependend of the training set
    unsigned int hiddenNeuronX      = 1;
    unsigned int hiddenNeuronY      = 4;
    unsigned int outputNeuron       = outputSet[0].size();    //Makes the amount of outputs dependent of the training set
    bool enableBias                 = true;
    bool enableAverage              = false;
    Activation activation           = Activation::Gaussian;


    BackpropNet *net = new BackpropNet(netID,
                                       inputNeurons,
                                       hiddenNeuronX,
                                       hiddenNeuronY,
                                       outputNeuron,
                                       enableBias,
                                       enableAverage,
                                       activation); //Makes the Net object

    net->loadFromNetFile();
    net->set_mutationFactor(0.1);
    net->updateNetConfiguration();
    net->saveToNetFile();


    std::vector<double> genom;
    std::vector<double> output;
    printf("net done, press enter\n");
    unsigned int counter =0;
    unsigned int saveCounter = 0;
    unsigned int saves = 10;
    double averageError = 0;



    system("cls");
    unsigned long learningSteps = 0;
    clock_t startTime = clock();
    t1 = std::chrono::high_resolution_clock::now();
    double averageCalcTime = 0;

    while(true)
    {

        if(net->get_errorAmount() != 0)
        {
            ErrorList errors = net->get_errorList();
            for(int error=0; error<errors.size(); error++)
            {
                std::cout << "Error: "<<errors[error].toQString().toStdString();
            }
            net->clearErrors();
            getchar();
        }


        net->set_input(trainingsSet[counter]);       // Sets the input of the net with the trainingset [counter]
        net->run();
        averageCalcTime = averageCalcTime*0.8+0.2*net->get_runtime();
        output = net->get_output();                  // Calculates the output std::vector and returns it
        net->set_expected(outputSet[counter]);       // Tells the net the right results

        averageError += abs(net->get_netError());    //The net calculates the error of netprediction and expected output
                                                //Saving only the positive value of the error to stop the training later when the error is low enough
        net->learn();                            //Improve the net
        learningSteps++;                        //Adding one training cycle

        if(isnan(averageError))
        {
            qDebug() << "nan";
        }

        counter++;                              //counts to the next trainingset
        saveCounter++;

        if(counter >= trainingsSet.size())
        {
            counter = 0;
            averageError /= trainingsSet.size(); //takes the average error of the whole training set
            if(saveCounter > saves)
            {
                cmdXY(0,0);  // Sets the cursor pos of the console
                saveCounter = 0;
                saves+=100; //spam the console in the beginning and later no more
                printf("error: %.5f\n",averageError);   //Prints the error
                printf("steps: %i\n",learningSteps);    //Prints the learn cyles

                logfile = fopen("score.csv","a");           //Saves the error in the file: score.csv
                fprintf(logfile,"%.5f;\n",averageError);    //
                fclose(logfile);                            //



                net->saveToNetFile();                        //Save the genom

                logGenom(net->get_genom());                      //Saves all weights of the net in: genom.csv so you can track the weights over the time of improvement
               // system("cls");
            }
            if(averageError < 0.0005 || learningSteps > 1000000)//Learn until the error is below 0.005 or learning cycles are more then 1000000
            {
                t2 = std::chrono::high_resolution_clock::now();
                time_span = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);
                cmdXY(0,0);  // Sets the cursor pos of the console
                saveCounter = 0;
                saves+=100; //spam the console in the beginning and later no more
                printf("error: %.5f\n",averageError);   //Prints the error
                printf("steps: %i\n",learningSteps);    //Prints the learn cyles

                logfile = fopen("score.csv","a");           //Saves the error in the file: score.csv
                fprintf(logfile,"%.5f;\n",averageError);    //
                fclose(logfile);                            //



                net->saveToNetFile();                        //Save the genom

                logGenom(net->get_genom());
                qDebug() << "Learning time: "<< time_span.count() << "sec.\t Average net calculation Time: "<<averageCalcTime<<"sec.";
                getchar();
                netFinished(net);
            }
            averageError = 0;

        }
    }
    return a.exec();
}
void printNet(Net *net)
{
    for(unsigned int b=0; b<net->get_neurons(); b++)
    {
    QStringList    list = net->get_ptr_neuron_viaID(b)->toStringList();
    for(int a=0; a<list.size(); a++)
    {
        printf(list[a].toStdString().c_str());
    }
    printf("------------------------------------\n");
    }
   /* printf("=================================================================================\n");
    printf("Input neurons:\n");
    printf("------------------\n");
    for(unsigned int y=0; y<net->hiddenNeuronsY(); y++)
    {
            printf("\tneuron      y: %i \t|\tinput\t|\tweight\t|\toutput\t|\n",y);
        for(unsigned int i=0; i<net->hiddenNeuron(0,y)->inputs(); i++)
        {
            printf("\t                \t|\t %.2f \t|\t %.2f \t|\t      \t|\n",net->hiddenNeuron(0,y)->input(i),net->hiddenNeuron(0,y)->weight(i));
        }
            printf("\t                \t|\t      \t|\t      \t|\t %.2f \t|\n",net->hiddenNeuron(0,y)->output());
            printf("---------------------------------------------------------------------------------\n");
    }
    printf("=================================================================================\n");
    printf("Hidden neurons:\n");
    printf("------------------\n");
    for(unsigned int x=1; x<net->hiddenNeuronsX(); x++)
    {
        for(unsigned int y=0; y<net->hiddenNeuronsY(); y++)
        {
                printf("\tneuron x: %i y: %i \t|\tinput\t|\tweight\t|\toutput\t|\n",x,y);
            for(unsigned int b=0; b<net->hiddenNeuron(x,y)->inputs(); b++)
            {
                printf("\t                \t|\t %.2f \t|\t %.2f \t|\t      \t|\n",net->hiddenNeuron(x,y)->input(b),net->hiddenNeuron(x,y)->weight(b));
            }
                printf("\t                \t|\t      \t|\t      \t|\t %.2f \t|\n",net->hiddenNeuron(x,y)->output());
                printf("---------------------------------------------------------------------------------\n");
        }
    }
    printf("=================================================================================\n");
    printf("Output neurons:\n");
    printf("------------------\n");
    for(unsigned int y=0; y<net->outputNeurons(); y++)
    {
        printf("\tneuron      y: %i \t|\tinput\t|\tweight\t|\toutput\t|\n",y);
        for(unsigned int b=0; b<net->outputNeuron(y)->inputs(); b++)
        {
            printf("\t                \t|\t %.2f \t|\t %.2f \t|\t      \t|\n",net->outputNeuron(y)->input(b),net->outputNeuron(y)->weight(b));
        }
            printf("\t                \t|\t      \t|\t      \t|\t %.2f \t|\n",net->output(y));
            printf("---------------------------------------------------------------------------------\n");
    }
    printf("=================================================================================\n");
*/
}
void setupTrainingSet()
{
    //              INPUT VALUES             EXPECTED OUTPUT
    trainingsSet.push_back({0,0});   outputSet.push_back({0});
    trainingsSet.push_back({0,1});   outputSet.push_back({1});
    trainingsSet.push_back({1,0});   outputSet.push_back({1});
    trainingsSet.push_back({1,1});   outputSet.push_back({0});
}
void netFinished(BackpropNet *net)
{
    std::vector<double*> *genom;
    std::vector<double> output;

    genom = net->get_ptr_genom();
    system("cls");
    printf("Genom: \n | ");
    for(unsigned int a=0; a<(*genom).size(); a++)
    {
        printf("%.3f\t|\t",*(*genom)[a]);
        if((a+1)%5 == 0)
        {
            printf("\n | ");
        }
    }
    printf("\n=================================================================================\n\n");
    while(true)
    {
        for(unsigned int counter=0; counter<trainingsSet.size(); counter++)
        {
            cmdXY(0,(*genom).size() / 5 +4);
            net->set_input(trainingsSet[counter]);
            net->run();
            output = net->get_output();

            printf("inputs are:\t");
            for(unsigned int a=0; a<trainingsSet[counter].size(); a++)
            {
                printf("%.3f | ",trainingsSet[counter][a]);
            }
            printf("\noutputs are:\t");
            for(unsigned int a=0; a<output.size(); a++)
            {
                printf("%.3f | ",output[a]);
            }
            printf("\n           \t=====");
            printf("\n\n\n=================================================================================\n");
            printNet(net);
            printf("\n=================================================================================\n");
            getchar();
        }
    }
}
void cmdXY(unsigned int x,unsigned int y)
{
    HANDLE hConsole_c = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos = {static_cast<short>(x), static_cast<short>(y)};
    SetConsoleCursorPosition(hConsole_c,pos);
}
void logGenom(std::vector<double> genom)
{
    genomlogFile = fopen("genom.csv","a");
    for(unsigned int a=0; a<genom.size(); a++)
    {
        fprintf(genomlogFile,"%.5f;",genom[a]);
    }
    fprintf(genomlogFile,"\n");
    fclose(genomlogFile);
}

#include "net.h"


Net::Net()
{
    Net(2,1,2,1);
}

Net::Net(unsigned int inputs,
    unsigned int hiddenX,
    unsigned int hiddenY,
    unsigned int outputs)
{
    Net(inputs,hiddenX,hiddenY,outputs,true,false,Activation::Sigmoid);
}

Net::Net(unsigned int inputs,
    unsigned int hiddenX,
    unsigned int hiddenY,
    unsigned int outputs,
    bool enableBias,
    bool enableAverage,
    Activation func)
{
    _updateNetConfiguration = true;
    _activationFunction = Activation::Sigmoid;
    _inputs = NET_MIN_INPUTNEURONS;
    _hiddenX = NET_MIN_HIDDENNEURONS_X;
    _hiddenY = NET_MIN_HIDDENNEURONS_Y;
    _outputs = NET_MIN_OUTPUTNEURONS;
    _bias = true;
    _enableAverage = false;
    updateNetConfiguration();
    activationFunction(func);
    this->outputNeurons(outputs);
    this->hiddenNeuronsY(hiddenY);
    this->hiddenNeuronsX(hiddenX);
    this->inputNeurons(inputs);
    this->bias(enableBias);
    this->enableAverage(enableAverage);
    biasValue(1.0);

    time_t timer;
    time(&timer);
    struct tm *ti = localtime(&timer);
    _randEngine = std::default_random_engine(rand()%100 + ti->tm_hour+ti->tm_min+ti->tm_sec);
}

Net::~Net()
{

}

void                Net::inputNeurons(unsigned int inputs)
{
    if(inputs < NET_MIN_INPUTNEURONS || inputs > NET_MAX_INPUTNEURONS)
    {
        throw std::runtime_error("NET::inputNeurons("+std::to_string(inputs)+") ERROR: Parameter 0 is out of range. Min: "+
                                 std::to_string(NET_MIN_INPUTNEURONS)+" Max: "+std::to_string(NET_MAX_INPUTNEURONS));
    }
    if(inputs != _inputs)
    {
        _update = true;
        _updateNetConfiguration = true;
        _inputs = inputs;
        updateNetConfiguration();
    }
}
unsigned int        Net::inputNeurons()
{
    return _inputs;
}
void                Net::hiddenNeuronsX(unsigned int hiddenX)
{
    if(hiddenX < NET_MIN_HIDDENNEURONS_X || hiddenX > NET_MAX_HIDDENNEURONS_X)
    {
        throw std::runtime_error("NET::hiddenNeuronsX("+std::to_string(hiddenX)+") ERROR: Parameter 0 is out of range. Min: "+
                                 std::to_string(NET_MIN_HIDDENNEURONS_X)+" Max: "+std::to_string(NET_MAX_HIDDENNEURONS_X));
    }
    if(hiddenX != _hiddenX)
    {
        _update  = true;
        _updateNetConfiguration = true;
        _hiddenX = hiddenX;
        updateNetConfiguration();
    }
}
unsigned int        Net::hiddenNeuronsX()
{
    return _hiddenX;
}
void                Net::hiddenNeuronsY(unsigned int hiddenY)
{
    if(hiddenY < NET_MIN_HIDDENNEURONS_Y || hiddenY > NET_MAX_HIDDENNEURONS_Y)
    {
        throw std::runtime_error("NET::hiddenNeuronsY("+std::to_string(hiddenY)+") ERROR: Parameter 0 is out of range. Min: "+
                                 std::to_string(NET_MIN_HIDDENNEURONS_Y)+" Max: "+std::to_string(NET_MAX_HIDDENNEURONS_Y));
    }
    if(hiddenY != _hiddenY)
    {
        _update  = true;
        _updateNetConfiguration = true;
        _hiddenY = hiddenY;
        updateNetConfiguration();
    }
}
unsigned int        Net::hiddenNeuronsY()
{
    return _hiddenY;
}
void                Net::outputNeurons(unsigned int outputs)
{
    if(outputs < NET_MIN_OUTPUTNEURONS || outputs > NET_MAX_OUTPUTNEURONS)
    {
        throw std::runtime_error("NET::outputNeurons("+std::to_string(outputs)+") ERROR: Parameter 0 is out of range. Min: "+
                                 std::to_string(NET_MIN_OUTPUTNEURONS)+" Max: "+std::to_string(NET_MAX_OUTPUTNEURONS));
    }
    if(outputs != _outputs)
    {
        _update  = true;
        _outputs = outputs;
        //setupNeurons();
        updateNetConfiguration();
    }
}
unsigned int        Net::outputNeurons()
{
    return _outputs;
}

void                Net::bias(bool enableBias)
{
    if(enableBias != _bias)
    {
        _update = true;
        _updateNetConfiguration = true;
        _bias   = enableBias;
        updateNetConfiguration();
    }
}
bool                Net::bias()
{
    return _bias;
}
void                Net::enableAverage(bool enableAverage)
{
    if(enableAverage != _enableAverage)
    {
        _update         = true;
        _enableAverage  = enableAverage;
        if(!_noHiddenLayer)
        {
            for(unsigned int x=0; x<_hiddenX; x++)
            {
                for(unsigned int y=0; y<_hiddenY; y++)
                {
                    _hiddenNeuronList[x][y].enableAverage(_enableAverage);
                }
            }
        }
        for(unsigned int a=0; a<_outputs; a++)
        {
            _outputNeuronList[a].enableAverage(_enableAverage);
        }
    }
}
bool                Net::enableAverage()
{
    return _enableAverage;
}
void                Net::biasValue(float value)
{
    _update     = true;
    _biasValue  = value;
}
float               Net::biasValue()
{
    return _biasValue;
}
void                Net::activationFunction(Activation func)
{
    _activationFunction = func;
    _update             = true;
    if(!_noHiddenLayer)
    {
        for(unsigned int x=0; x<_hiddenX; x++)
        {
            for(unsigned int y=0; y<_hiddenY; y++)
            {
                _hiddenNeuronList[x][y].activationFunction(_activationFunction);
            }
        }
    }
    for(unsigned int a=0; a<_outputs; a++)
    {
        _outputNeuronList[a].activationFunction(_activationFunction);
    }
}
Activation          Net::activationFunction()
{
    return _activationFunction;
}
bool                Net::noHiddenLayer()
{
    return _noHiddenLayer;
}

void                Net::randomGenom()
{
    unsigned int genomsize	= 0;
    if(_noHiddenLayer)
    {
        genomsize = (_inputs+(unsigned int)_bias) * _outputs;
    }
    else
    {
        genomsize = (_inputs+(unsigned int)_bias) * _hiddenY + (_hiddenY+(unsigned int)_bias) * _hiddenY * (_hiddenX-1) + (_hiddenY+(unsigned int)_bias) * _outputs;
    }
    if(genomsize == 0)
    {
        throw std::runtime_error("Net::randomGenom() ERROR: genomsize can't be zero (0)\n");
    }
    _genom = std::vector<float>(genomsize,0);
    for(unsigned int a=0; a<genomsize; a++)
    {
        _genom[a] = (float)(_randEngine()%2000)/1000 - (float)1;
    }
    setGenomToNeuron();
}
void                Net::genom(std::vector<float> genom)
{
    if(genom.size() != _genom.size())
    {
        throw std::runtime_error("Net::genom(std::vector<float>) ERROR: parameter 0 has the wrong array size: "+std::to_string(genom.size())+" array size should by "+std::to_string(_genom.size())+"\n");
    }
    _genom = genom;
    setGenomToNeuron();
}
std::vector<float>  Net::genom()
{
    getGenomFromNeuron();
    return _genom;
}
unsigned int        Net::genomsize()
{
    getGenomFromNeuron();
    return _genom.size();
}

void                Net::input(unsigned int input, float signal)
{
    if(input > _inputs-1)
    {
        throw std::runtime_error("Net::input(unsigned int , float) "+error_paramOutOfRange(0,input,_inputs-1));
    }
    _update = true;
    if(_noHiddenLayer)
    {
        for(unsigned int y=0; y<_outputNeuronList.size(); y++)
        {
            try {
                _outputNeuronList[y].input(input,signal);
            } catch (std::runtime_error *e) {
                std::string error = "Net::input(unsigned int ["+std::to_string(input)+"] , float ["+std::to_string(signal)+"]) ERROR:\n";
                            error+= e->what();
                throw std::runtime_error(error);
            }
        }
    }
    else
    {
        for(unsigned int y=0; y<_hiddenNeuronList[0].size(); y++)
        {

            try {
                _hiddenNeuronList[0][y].input(input,signal);
            } catch (std::runtime_error *e) {
                std::string error = "Net::input(unsigned int , float) ERROR:\n";
                            error+= e->what();
                throw std::runtime_error(error);
            }
        }
    }
}
float               Net::input(unsigned int input)
{
    if(input > _inputs-1)
    {
        throw std::runtime_error("Net::input(unsigned int) "+error_paramOutOfRange(0,input,_inputs-1));
    }
    float inp = 0;
    if(_noHiddenLayer)
    {
        try {
            inp = _outputNeuronList[0].input(input);
        } catch (std::runtime_error *e) {
            std::string error = "Net::input(unsigned int ["+std::to_string(input)+"]) ERROR: output neuron: 0 input: "+std::to_string(input)+"\n";
                        error+= e->what();
            throw std::runtime_error(error);
        }
    }
    else
    {
        try {
            inp = _hiddenNeuronList[0][0].input(input);
        } catch (std::runtime_error *e) {
            std::string error = "Net::input(unsigned int ["+std::to_string(input)+"]) ERROR: hidden neuron X: 0 hidden neuron Y: 0 input: "+std::to_string(input)+"\n";
                        error+= e->what();
            throw std::runtime_error(error);
        }
    }
    return inp;
}
void                Net::input(std::vector<float> inputList)
{
    if(inputList.size() != _inputs)
    {
        throw std::runtime_error("net::input(std::vector<float>) ERROR: parameter 0 , size of the array is wrong: "+std::to_string(inputList.size())+" size has to be: "+std::to_string(_inputs)+"\n");
    }
    _update = true;
    if(_bias)
    {
        inputList.push_back(_biasValue);
    }
    if(_noHiddenLayer)
    {
        for(unsigned int y=0; y<_outputNeuronList.size(); y++)
        {
            try {
                _outputNeuronList[y].input(inputList);
            } catch (std::runtime_error *e) {
                std::string error = "net::input(std::vector<float>) ERROR: output neuron Y: "+std::to_string(y)+"\n";
                            error+= e->what();
                throw std::runtime_error(error);
            }
        }
    }
    else
    {
        for(unsigned int y=0; y<_hiddenNeuronList[0].size(); y++)
        {
            try {
                _hiddenNeuronList[0][y].input(inputList);
            } catch (std::runtime_error *e) {
                std::string error = "net::input(std::vector<float>) ERROR: hidden neuron X: 0 hidden neuron Y: "+std::to_string(y)+"\n";
                            error+= e->what();
                throw std::runtime_error(error);
            }
        }
    }
}
std::vector<float>  Net::input()
{
    if(_noHiddenLayer)
    {
        return _outputNeuronList[0].input();
    }
    else
    {
        return _hiddenNeuronList[0][0].input();
    }
}

float               Net::hidden(unsigned int hiddenX, unsigned int hiddenY)
{
    if(_noHiddenLayer)
    {
        throw std::runtime_error("Net::hidden(unsigned int ["+std::to_string(hiddenX)+"] , unsigned int ["+std::to_string(hiddenY)+"] ) ERROR: the network has no hidden layer\n");
    }
    if(hiddenX > _hiddenX-1)
    {
        throw std::runtime_error("Net::hidden(unsigned int ["+std::to_string(hiddenX)+"] , unsigned int ["+std::to_string(hiddenY)+"] ) "+error_paramOutOfRange(0,hiddenX,_hiddenX-1));
    }
    if(hiddenY > _hiddenY-1)
    {
        throw std::runtime_error("Net::hidden(unsigned int ["+std::to_string(hiddenX)+"] , unsigned int ["+std::to_string(hiddenY)+"] ) "+error_paramOutOfRange(1,hiddenY,_hiddenY-1));
    }
    run();
    return _hiddenNeuronList[hiddenX][hiddenY].output();
}
std::vector<float>  Net::hiddenX(unsigned int hiddenX)// |    Alle in einer Spalte
{
    if(_noHiddenLayer)
    {
        throw std::runtime_error("Net::hiddenX(unsigned int ["+std::to_string(hiddenX)+"] ) ERROR: the network has no hidden layer\n");
    }
    if(hiddenX > _hiddenX-1)
    {
        throw std::runtime_error("Net::hiddenX(unsigned int ["+std::to_string(hiddenX)+"] ) "+error_paramOutOfRange(0,hiddenX,_hiddenX-1));
    }
    run();
    std::vector<float> ret(_hiddenY,0);
    for(unsigned int y=0; y<_hiddenY; y++)
    {
        ret[y] = _hiddenNeuronList[hiddenX][y].output();
    }
    return ret;
}
std::vector<float>  Net::hiddenY(unsigned int hiddenY)// --   Alle in einer Reihe
{
    if(_noHiddenLayer)
    {
        throw std::runtime_error("Net::hiddenY(unsigned int ["+std::to_string(hiddenY)+"] ) ERROR: the network has no hidden layer\n");
    }
    if(hiddenY > _hiddenY-1)
    {
        throw std::runtime_error("Net::hiddenY(unsigned int ["+std::to_string(hiddenY)+"] ) "+error_paramOutOfRange(0,hiddenY,_hiddenY-1));
    }
    run();
    std::vector<float> ret(_hiddenX,0);
    for(unsigned int x=0; x<_hiddenX; x++)
    {
        ret[x] = _hiddenNeuronList[x][hiddenY].output();
    }
    return ret;
}

Neuron              Net::hiddenNeuron(unsigned int hiddenX, unsigned int hiddenY)
{
    if(_noHiddenLayer)
    {
        throw std::runtime_error("Net::hiddenNeuron(unsigned int ["+std::to_string(hiddenX)+"] , unsigned int ["+std::to_string(hiddenY)+"] ) ERROR: the network has no hidden layer\n");
    }
    if(hiddenX > _hiddenX-1)
    {
        throw std::runtime_error("Net::hidden(unsigned int ["+std::to_string(hiddenX)+"] , unsigned int ["+std::to_string(hiddenY)+"] ) "+error_paramOutOfRange(0,hiddenX,_hiddenX-1));
    }
    if(hiddenY > _hiddenY-1)
    {
        throw std::runtime_error("Net::hidden(unsigned int ["+std::to_string(hiddenX)+"] , unsigned int ["+std::to_string(hiddenY)+"] ) "+error_paramOutOfRange(0,hiddenY,_hiddenY-1));
    }
    run();
    return _hiddenNeuronList[hiddenX][hiddenY];
}
std::vector<Neuron> Net::hiddenNeuronX(unsigned int hiddenX) // |    Alle in einer Spalte
{
    if(_noHiddenLayer)
    {
        throw std::runtime_error("Net::hiddenNeuronX(unsigned int ["+std::to_string(hiddenX)+"] ) ERROR: the network has no hidden layer\n");
    }
    if(hiddenX > _hiddenX-1)
    {
        throw std::runtime_error("Net::hiddenNeuronX(unsigned int ["+std::to_string(hiddenX)+"] ) "+error_paramOutOfRange(0,hiddenX,_hiddenX-1));
    }
    run();
    return _hiddenNeuronList[hiddenX];
}
std::vector<Neuron> Net::hiddenNeuronY(unsigned int hiddenY)// --   Alle in einer Reihe
{
    if(_noHiddenLayer)
    {
        throw std::runtime_error("Net::hiddenNeuronY(unsigned int ["+std::to_string(hiddenY)+"] ) ERROR: the network has no hidden layer\n");
    }
    if(hiddenY > _hiddenY-1)
    {
        throw std::runtime_error("Net::hiddenNeuronY(unsigned int ["+std::to_string(hiddenY)+"] ) "+error_paramOutOfRange(0,hiddenY,_hiddenY-1));
    }
    run();
    std::vector<Neuron> ret;
    for(unsigned int x=0; x<_hiddenX; x++)
    {
        ret.push_back(_hiddenNeuronList[x][hiddenY]);
    }
    return ret;
}
std::vector<std::vector<Neuron> > Net::hiddenNeuron()
{
    if(_noHiddenLayer)
    {
        throw std::runtime_error("Net::hiddenNeuron() ERROR: the network has no hidden layer\n");
    }
    run();
    return _hiddenNeuronList;
}
Neuron              Net::outputNeuron(unsigned int output)
{
    if(output > _outputs-1)
    {
        throw std::runtime_error("Net::outputNeuron(unsigned int ["+std::to_string(output)+"] ) "+error_paramOutOfRange(0,output,_outputs-1));
    }
    run();
    return _outputNeuronList[output];
}
std::vector<Neuron> Net::outputNeuron()
{
    return _outputNeuronList;
}


float               Net::output(unsigned int output)
{
    if(output > _outputs)
    {
        throw std::runtime_error("Net::output(unsigned int ["+std::to_string(output)+"] ) "+error_paramOutOfRange(0,output,_outputs-1));
    }
    run();
    return _outputNeuronList[output].output();
}
std::vector<float>  Net::output()
{
    run();
    std::vector<float> ret(_outputs,0);
    for(unsigned int y=0; y<_outputs; y++)
    {
        ret[y] = _outputNeuronList[y].output();
    }
    return ret;
}

void                Net::run()
{
    if(_update)
    {
        if(!_noHiddenLayer)
        {
            for(unsigned int x=0; x<_hiddenNeuronList.size()-1; x++)  //For every hidden layer in x ---> direction
            {
                for(unsigned int y=0; y<_hiddenNeuronList[x+1].size(); y++)
                {
                    for(unsigned int y_layerBefore=0; y_layerBefore<_hiddenNeuronList[x].size(); y_layerBefore++)
                    {
                        _hiddenNeuronList[x+1][y].input(y_layerBefore,_hiddenNeuronList[x][y_layerBefore].output());
                    }
                    if(_bias)
                    {
                        _hiddenNeuronList[x+1][y].input(_hiddenY,_biasValue);//_hiddenY because pos begins by 0 for the value 1 and with the bias we have to calc the last element + 1
                    }
                }
            }
            for(unsigned int outY=0; outY<_outputs; outY++)
            {
                for(unsigned int hidY=0; hidY<_hiddenY; hidY++)
                {
                    _outputNeuronList[outY].input(hidY,_hiddenNeuronList[_hiddenX-1][hidY].output());
                }
                if(_bias)
                {
                    _outputNeuronList[outY].input(_hiddenY,_biasValue);//_hiddenY because pos begins by 0 for the value 1 and with the bias we have to calc the last element + 1
                }
            }
        }
        for(unsigned int outY=0; outY<_outputs; outY++)
        {
            _outputNeuronList[outY].run();
        }
        _update = false;
    }
}
void                Net::updateNetConfiguration()
{
    _update = true;
    if(_hiddenX == 0 || _hiddenY == 0)
    {
        _noHiddenLayer = true;
    }
    else
    {
        _noHiddenLayer = false;
    }
    _hiddenNeuronList = std::vector<std::vector<Neuron> >();
    _outputNeuronList = std::vector<Neuron>();
    for(unsigned int y=0; y<_outputs; y++)
    {
        _outputNeuronList.push_back(Neuron(_hiddenY+(unsigned int)_bias));
        _outputNeuronList[y].inputs(_hiddenY+(unsigned int)_bias);
        _outputNeuronList[y].activationFunction(_activationFunction);
        _outputNeuronList[y].enableAverage(_enableAverage);
    }
    for(unsigned int x=0; x<_hiddenX; x++)
    {
        _hiddenNeuronList.push_back(std::vector<Neuron>());
        for(unsigned int y=0; y<_hiddenY; y++)
        {
            _hiddenNeuronList[x].push_back(Neuron(_hiddenY+(unsigned int)_bias));
            _hiddenNeuronList[x][y].inputs(_hiddenY+(unsigned int)_bias);
            _hiddenNeuronList[x][y].activationFunction(_activationFunction);
            _hiddenNeuronList[x][y].enableAverage(_enableAverage);
        }
    }
    if(_noHiddenLayer)
    {
        for(unsigned int y=0; y<_outputs; y++)
        {
            _outputNeuronList[y].inputs(_inputs+(unsigned int)_bias);
        }
    }
    else
    {
        for(unsigned int y=0; y<_hiddenY; y++)
        {
            _hiddenNeuronList[0][y].inputs(_inputs+(unsigned int)_bias);
        }
    }
    _updateNetConfiguration = false;
    randomGenom();
}

void                Net::setGenomToNeuron()
{
    _update = true;
    unsigned int genomsize = 0;
    for(unsigned int x=0; x<_hiddenNeuronList.size(); x++)
    {
        for(unsigned int y=0; y<_hiddenNeuronList[x].size(); y++)
        {
            genomsize += _hiddenNeuronList[x][y].inputs();
        }
    }
    for(unsigned int y=0; y<_outputNeuronList.size(); y++)
    {
        genomsize += _outputNeuronList[y].inputs();
    }
    if(genomsize != _genom.size())
    {
        std::string error = "Net::setGenomToNeuron() ERROR: geonomsize is wrong\n";
                    error+= "genomsize is: "+std::to_string(_genom.size()) + " but the Net has only place for an amount of: "+ std::to_string(genomsize) + " weights.\n";
                    error+= "Check your net configuration:\n";
                    error+= "\tInput Neurons Y:\t"+std::to_string(_inputs)+"\n";
                    error+= "\tHiddenNeurons X:\t"+std::to_string(_hiddenX)+"\n";
                    error+= "\tHiddenNeurons Y:\t"+std::to_string(_hiddenY)+"\n";
                    error+= "\tOutputNeurons Y:\t"+std::to_string(_outputs)+"\n";

        throw std::runtime_error(error);
    }
    unsigned int weightPos = 0;
    if(!_noHiddenLayer)
    {
        for(unsigned int x=0; x<_hiddenNeuronList.size(); x++)
        {
            for(unsigned int y=0; y<_hiddenNeuronList[x].size(); y++)
            {
                for(unsigned int w=0; w<_hiddenNeuronList[x][y].inputs(); w++)
                {
                    try {
                        _hiddenNeuronList[x][y].weight(w,_genom[weightPos]);
                        weightPos++;
                    } catch (std::runtime_error *e) {
                        std::string error = "Net::setGenomToNeuron() ERROR: On setting the weight for the hidden neuron X: "+std::to_string(x)+" Y: "+std::to_string(y)+" weight: "+std::to_string(w) + " ";
                                    error +="with the value: "+std::to_string(_genom[weightPos]) + " from the genom on the arrayPos: "+std::to_string(weightPos)+"\n";
                                    error +=e->what();
                        throw std::runtime_error(error);
                    }
                }
            }
        }
    }
    for(unsigned int y=0; y<_outputNeuronList.size(); y++)
    {
        for(unsigned int w=0; w<_outputNeuronList[y].inputs(); w++)
        {
            try {
                _outputNeuronList[y].weight(w,_genom[weightPos]);
                weightPos++;
            } catch (std::runtime_error *e) {
                std::string error = "Net::setGenomToNeuron() ERROR: On setting the weight for the output neuron Y: "+std::to_string(y)+" weight: "+std::to_string(w) + " ";
                            error +="with the value: "+std::to_string(_genom[weightPos]) + " from the genom on the arrayPos: "+std::to_string(weightPos)+"\n";
                            error +=e->what();
                throw std::runtime_error(error);
            }
        }
    }
}
void                Net::getGenomFromNeuron()
{
    _genom = std::vector<float>();
    if(!_noHiddenLayer)
    {
        for(unsigned int x=0; x<_hiddenNeuronList.size(); x++)
        {
            for(unsigned int y=0; y<_hiddenNeuronList[x].size(); y++)
            {
                for(unsigned int w=0; w<_outputNeuronList[y].inputs(); w++)
                {
                    try {
                        _genom.push_back(_hiddenNeuronList[x][y].weight(w));
                    } catch (std::runtime_error *e) {
                        std::string error = "Net::getGenomFromNeuron() ERROR: On getting the weight for the hidden neuron X: "+std::to_string(x)+" Y: "+std::to_string(y)+" weight: "+std::to_string(w) + " ";
                                    error +="current genomsize: "+std::to_string(_genom.size())+"\n";
                                    error +=e->what();
                    }
                }
            }
        }
    }
    for(unsigned int y=0; y<_outputNeuronList.size(); y++)
    {
        for(unsigned int w=0; w<_outputNeuronList[y].inputs(); w++)
        {
            try {
                _genom.push_back(_outputNeuronList[y].weight(w));
            } catch (std::runtime_error *e) {
                std::string error = "Net::getGenomFromNeuron() ERROR: On getting the weight for the output neuron Y: "+std::to_string(y)+" weight: "+std::to_string(w) + " ";
                            error +="current genomsize: "+std::to_string(_genom.size())+"\n";
                            error +=e->what();
            }
        }
    }
}



//----------ERROR

std::string Net::error_paramOutOfRange(unsigned int paramPos,std::string value, std::string max)
{
    return " ERROR: parameter "+std::to_string(paramPos)+" is out of range: "+value+" maximum is: "+max+"\n";
}
std::string Net::error_paramOutOfRange(unsigned int paramPos,unsigned int value, unsigned int max)
{
    return error_paramOutOfRange(paramPos,std::to_string(value),std::to_string(max));
}
std::string Net::error_paramOutOfRange(unsigned int paramPos,int value, int max)
{
    return error_paramOutOfRange(paramPos,std::to_string(value),std::to_string(max));
}
std::string Net::error_paramOutOfRange(unsigned int paramPos,float value, float max)
{
    return error_paramOutOfRange(paramPos,std::to_string(value),std::to_string(max));
}

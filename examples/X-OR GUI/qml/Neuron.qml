import QtQuick 2.0

Rectangle {
    property int d: 20
    property real neuronValue: 0
    property int neuronID: 0
    property int typeId:0
    property int type: 0
    onXChanged: {
        dockingPoint()
    }
    onYChanged: {
        dockingPoint()
    }
    function dockingPoint(){
        var xInput
        var yInput
        var xOutput
        var yOutput
        xInput=0+x
        xOutput=d+x
        yInput=d/2+y
        yOutput=d/2+y
        if(type===inputType){
            inputConXOutput[typeId]=xOutput
            inputConYOutput[typeId]=yOutput
        }
        else if(type===biasType){
            biasConXOutput[typeId]=xOutput
            biasConYOutput[typeId]=yOutput
        }
        else if(type===hiddenType){
            hiddenConXInput[typeId]=xInput
            hiddenConYInput[typeId]=yInput
            hiddenConXOutput[typeId]=xOutput
            hiddenConYOutput[typeId]=yOutput
        }
        else if(type===outputType){
            outputConXInput[typeId]=xInput
            outputConYInput[typeId]=yInput
        }
    }
    property string neuronColor:if(neuronValue<0){
                                          return"darkred";
                                            }
                                      else
                                            {
                                          return"darkgreen";
                                            }
    width: d
    height: d
    border.width: d*0.05
    border.color: "black"
    radius: d/2
    color: if(neuronValue!=0){
                return Qt.lighter(neuronColor,(1-Math.abs(neuronValue))*4);
         }
         else{
             return "white"
         }
    Text {
        horizontalAlignment: Text.AlignHCenter
        text: if(type>0) return  "ID:"+neuronID+"\nValue: "+Math.round(neuronValue*10000)/10000
                else return "Value: "+Math.round(neuronValue*10000)/10000
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter

        color:if(Math.abs(neuronValue)>0.8){
                              return "white"
                          }
                          else{
                              return "black"
                          }
    }
}

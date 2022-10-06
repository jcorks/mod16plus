@:SES = import(module:'SES.Core');







@textarea = SES.Text.createArea();


// full GBA screen
textarea. = {
    widthChars  : 40,
    heightChars : 20,    
    editable    : true,
    scrollable  : true
};







SES.Input.addCallback(
    device:SES.Input.DEVICES.POINTER0,
    callback:::(event, x, y, button) {
        if (event == SES.Input.EVENTS.POINTER_BUTTON_DOWN) 
            SES.debug();
    }
);




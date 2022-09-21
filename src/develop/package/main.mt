@:SES = import(module:'SES.Core');




@textarea = SES.Text.createArea();


// full GBA screen
textarea. = {
    widthChars  : 40,
    heightChars : 20,    
    editable    : true
};



textarea.text = "return class(
    name: 'SES',
    
    define:::(this) {
        @:ATTRIBS = {
            UPDATERATE:  0,
            UPDATEFUNC:  1,
            RESOLUTION:  2,
            ADDALARM:    3,
            REMOVEALARM: 4,
        };
        
        @:RESOLUTION = {
            NES : 0, // 256 x 240       
            GBA : 1, // 240 x 160,
            GBC : 2, // 160 x 144,
            MD  : 3, // 320 x 224
        };


        @updateRate = 1 / 60; // how fast update should be called in the engine
        @updateFunc = ::{};
        @resolution = RESOLUTION.GBA;


        // before update is called: engine polls input         
        @update = ::{
            // dispatch events
            updateFunc();
        };
        // after update is called: backgrounds + sprites are posted to screen
        
        
        

        ses_native__engine_attrib(a:ATTRIBS.UPDATERATE, b:updateRate);
        ses_native__engine_attrib(a:ATTRIBS.UPDATEFUNC, b:update);
        ses_native__engine_attrib(a:ATTRIBS.RESOLUTION, b:resolution);
    
        this.interface = {
            Sprite    : {get ::<- Sprite},
            Palette   : {get ::<- Palette},
            Tile      : {get ::<- Tile},
            Input     : {get ::<- Input},
            Background: {get ::<- Background},
            Audio     : {get ::<- Audio},
            Text      : {get ::<- Text},

            RESOLUTION : RESOLUTION,

            resolution : {
                set ::(value => Number) {
                
                },
                get ::<- resolution
            },
            
    
            // add a function to call expireMS milliseconds later.
            // Only as resolute as the updateRate.
            // The alarm is removed after it expires.            
            addAlarm ::(expireMS => Number, callback => Function) {
                return ses_native__engine_attrib(a:ATTRIBS.ADDALARM, b:expireMS, c:callback);
            },
            
            
            // removes an alarm that is currently active, else does nothing.
            removeAlarm ::(id => Number) {
                ses_native__engine_attrib(a:ATTRIBS.REMOVEALARM, b:id);
            },
            
            

            
            
            

            
            updateRate : {
                set ::(value => Number) {
                    updateRate = value;
                    ses_native__engine_attrib(a:ATTRIBS.UPDATERATE, b:updateRate);                    
                },
                
                get ::<- updateRate
            },
            
            update : {
                set ::(value => Function) <- updateFunc = value,
                get ::<- updateFunc
            }
            
        };
    
    }
).new();";




SES.Input.addCallback(
    device:SES.Input.DEVICES.POINTER0,
    callback:::(event, x, y, button) {
        if (event == SES.Input.EVENTS.POINTER_SCROLL) ::<= {
            textarea.scrollX -= x;
            textarea.scrollY -= y;
        
        };

        if (event == SES.Input.EVENTS.POINTER_BUTTON_DOWN) ::<= {
            @:a = textarea.pixelCoordsToCursor(x, y);        
            textarea.moveCursor(x:a.x, y:a.y);
        };
    }
);

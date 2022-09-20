@:SES = import(module:'SES.Core');




@textarea = SES.createTextArea();


// full GBA screen
textarea.widthChars = 40;
textarea.heightChars = 20;

textarea.text = 
'

@:ses_native__sprite_attrib = getExternalFunction(name:"ses_native__sprite_attrib");
@:ses_native__engine_attrib = getExternalFunction(name:"ses_native__engine_attrib");
@:ses_native__palette_attrib = getExternalFunction(name:"ses_native__palette_attrib");
@:ses_native__tile_attrib = getExternalFunction(name:"ses_native__tile_attrib");
@:ses_native__input_attrib = getExternalFunction(name:"ses_native__input_attrib");
@:ses_native__audio_attrib = getExternalFunction(name:"ses_native__audio_attrib");
@:ses_native__bg_attrib = getExternalFunction(name:"ses_native__bg_attrib");

// query functions are necessary because they are (can be) pre-populated by the ROM.
@:ses_native__palette_query = getExternalFunction(name:"ses_native__palette_query");
@:ses_native__tile_query = getExternalFunction(name:"ses_native__tile_query");
@:ses_native__bg_query = getExternalFunction(name:"ses_native__bg_query");


// preset palettes are loaded from the rom
@:Palette = ::<= {
    @:hexToNum = {
        "0": 0,
        "1": 1,
        "2": 2,
        "3": 3,
        "4": 4,
        "5": 5,
        "6": 6,
        "7": 7,
        "8": 8,
        "9": 9,
        "a": 10,
        "b": 11,
        "c": 12,
        "d": 13,
        "e": 14,
        "f": 15,
        "A": 10,
        "B": 11,
        "C": 12,
        "D": 13,
        "E": 14,
        "F": 15
    };
    
    @:parseHex::(hi, lo) {
        return (hexToNum[lo] + hexToNum[hi]*16);
    };

    @:parseColor = ::(input){
        return match(input->type) {
          (Object): input,
          (String):
            [
                parseHex(hi:input->charAt(index:1), lo:input->charAt(index:2)) / 255, //r
                parseHex(hi:input->charAt(index:1), lo:input->charAt(index:2)) / 255, //g
                parseHex(hi:input->charAt(index:1), lo:input->charAt(index:2)) / 255, //b
            ]
        };
    };

    @:COLORS = {
        BACK: 0,
        MIDBACK: 1,
        MIDFRONT: 2,
        FRONT: 3
    };

    return class(
        name: "SES.Palette",
        
        
        define:::(this) {
            this.interface = {
                set::(
                    index => Number,
                    
                    // each can either be an array of rgb or a hex string prefixed with #
                    colors => Object
                ) {

                    @colorBack     = parseColor(input:colors[0]);
                    @colorMidBack  = parseColor(input:colors[1]);
                    @colorMidFront = parseColor(input:colors[2]);
                    @colorFront    = parseColor(input:colors[3]);

                    ses_native__palette_attrib(a:index, b:COLORS.BACK,     c:colorBack[0],     d:colorBack[1],     e:colorBack[2]);
                    ses_native__palette_attrib(a:index, b:COLORS.MIDBACK,  c:colorMidBack[0],  d:colorMidBack[1],  e:colorMidBack[2]);
                    ses_native__palette_attrib(a:index, b:COLORS.MIDFRONT, c:colorMidFront[0], d:colorMidFront[1], e:colorMidFront[2]);
                    ses_native__palette_attrib(a:index, b:COLORS.FRONT,    c:colorFront[0],    d:colorFront[1],    e:colorFront[2]);
                        
                },
                
                
                get::(
                    index => Number
                ) {
                    return [
                        [
                            ses_native__palette_query(a:index, b:COLORS.BACK, c:0),
                            ses_native__palette_query(a:index, b:COLORS.BACK, c:1),
                            ses_native__palette_query(a:index, b:COLORS.BACK, c:2)
                        ],
                        
                        [
                            ses_native__palette_query(a:index, b:COLORS.MIDBACK, c:0),
                            ses_native__palette_query(a:index, b:COLORS.MIDBACK, c:1),
                            ses_native__palette_query(a:index, b:COLORS.MIDBACK, c:2)
                        ],
                        
                        [
                            ses_native__palette_query(a:index, b:COLORS.MIDFRONT, c:0),
                            ses_native__palette_query(a:index, b:COLORS.MIDFRONT, c:1),
                            ses_native__palette_query(a:index, b:COLORS.MIDFRONT, c:2)
                        ],
                    
                        [
                            ses_native__palette_query(a:index, b:COLORS.FRONT, c:0),
                            ses_native__palette_query(a:index, b:COLORS.FRONT, c:1),
                            ses_native__palette_query(a:index, b:COLORS.FRONT, c:2)
                        ]
                    ];
                }
            };        
        }
    ).new();
};


';

SES.Input.addCallback(
    device:SES.Input.DEVICES.POINTER0,
    callback:::(event, x, y) {
        SES.Sprite.set(
            index: 2000,
            tile: '.'->charCodeAt(index:0),
            show:true,
            scaleX:1,
            scaleY:10,
            centerX: -3,
            centerY: -8,
            x: x,
            y: y,
            effect: SES.Sprite.EFFECTS.Color
        );     
    }
);

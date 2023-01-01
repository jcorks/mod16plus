@:class = import(module:'Matte.Core.Class');


@:ses_native__sprite_attrib = getExternalFunction(name:"ses_native__sprite_attrib");
@:ses_native__engine_attrib = getExternalFunction(name:"ses_native__engine_attrib");
@:ses_native__palette_attrib = getExternalFunction(name:"ses_native__palette_attrib");
@:ses_native__tile_attrib = getExternalFunction(name:"ses_native__tile_attrib");
@:ses_native__input_attrib = getExternalFunction(name:"ses_native__input_attrib");
@:ses_native__audio_attrib = getExternalFunction(name:"ses_native__audio_attrib");
@:ses_native__bg_attrib = getExternalFunction(name:"ses_native__bg_attrib");
@:ses_native__oscillator_attrib = getExternalFunction(name:"ses_native__oscillator_attrib");

// query functions are necessary because they are (can be) pre-populated by the ROM.
@:ses_native__palette_query = getExternalFunction(name:"ses_native__palette_query");
@:ses_native__tile_query = getExternalFunction(name:"ses_native__tile_query");

// preset palettes are loaded from the rom
@:Palette = ::<= {
    @:hexToNum = {
        '0': 0,
        '1': 1,
        '2': 2,
        '3': 3,
        '4': 4,
        '5': 5,
        '6': 6,
        '7': 7,
        '8': 8,
        '9': 9,
        'a': 10,
        'b': 11,
        'c': 12,
        'd': 13,
        'e': 14,
        'f': 15,
        'A': 10,
        'B': 11,
        'C': 12,
        'D': 13,
        'E': 14,
        'F': 15
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
                parseHex(hi:input->charAt(index:3), lo:input->charAt(index:4)) / 255, //g
                parseHex(hi:input->charAt(index:5), lo:input->charAt(index:6)) / 255, //b
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
        name: 'SES.Palette',
        
        
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


// preset tiles are loaded from the rom
@lockTiles = false;
@:Tile = ::<= {
    @:ATTRIBS = {
        BIND       : 0,
        SETTEXEL   : 1,
        UNBIND     : 2,
        COPY       : 3
    };

    return class(
        name: 'SES.Tile',
        
        define:::(this) {
            this.interface = {
            
                // data is a plain array of numbers, 0 - 4
                set ::(index => Number, data => Object) {
                    when(lockTiles && index < 128)  
                        error(detail:'Tiles 0 through 128 are reserved for the system.');
                    ses_native__tile_attrib(a:index, b:ATTRIBS.BIND);
                    [0, 64]->for(do:::(i) {
                        ses_native__tile_attrib(
                            a:i,
                            b:ATTRIBS.SETTEXEL,
                            c:data[i] => Number
                        );
                    });
                    ses_native__tile_attrib(a:index, b:ATTRIBS.UNBIND);

                },
                
                
                get ::(index => Number) {
                    @:out = [];
                    ses_native__tile_attrib(a:index, b:ATTRIBS.BIND);
                    [0, 64]->for(do:::(i) {
                        out->push(value:ses_native__tile_query(a:i));
                    });
                    ses_native__tile_attrib(a:index, b:ATTRIBS.UNBIND);
                    return out;
                },
                
                copy ::(to => Number, from => Number) {
                    ses_native__tile_attrib(a:to, b:ATTRIBS.BIND);
                    ses_native__tile_attrib(a:from, b:ATTRIBS.COPY);
                    ses_native__tile_attrib(a:to, b:ATTRIBS.UNBIND);
                }

            };
        }
    ).new();
};



@:Input = ::<= {
    @:DEVICES = {
        KEYBOARD: 0,
 
        // mouse + multitouch
        POINTER0: 1, 
        POINTER1: 2,
        POINTER2: 3,
        POINTER3: 4,
        
        
        
        GAMEPAD0: 5,
        GAMEPAD1: 6,
        GAMEPAD2: 7,
        GAMEPAD3: 8
    };

    @:EVENTS = {
        POINTER_MOTION : 0,
        KEYBOARD_TEXT : 1,
        KEY_DOWN : 2,
        POINTER_BUTTON_DOWN: 3,
        POINTER_BUTTON_UP: 4,
        POINTER_SCROLL: 5,
        KEY_UP : 6

    };
    
    @:POINTER_BUTTONS = {
        LEFT: 0,
        MIDDLE: 1,
        RIGHT: 2
    };
    
    // moreorless match SDLs
    @:KEYS = {
        UNKNOWN: 0,
        BACKSPACE: 8,
        TAB: 9,
        RETURN: 13,
        ESCAPE: 27,
        SPACE: 32,
        EXCLAM:33,
        QUOTEDBL:34,
        HASH:35,
        DOLLAR:36,
        PERCENT:37,
        AMPERSAND:38,
        QUOTE:39,
        LEFTPAREN:40,
        RIGHTPAREN:41,
        ASTERISK:42,
        PLUS:43,
        COMMA:44,
        MINUS:45,
        PERIOD:46,
        SLASH:47,
        KEY0:48,
        KEY1:49,
        KEY2:50,
        KEY3:51,
        KEY4:52,
        KEY5:53,
        KEY6:54,
        KEY7:55,
        KEY8:56,
        KEY9:57,
        COLON:58,
        SEMICOLON:59,
        LESS:60,
        EQUALS:61,
        GREATER:62,
        QUESTION:63,
        AT:64,
        LEFTBRACKET:91,
        BACKSLASH:92,
        RIGHTBRACKET:93,
        CARET:94,
        UNDERSCORE:95,
        BACKQUOTE:96,
        A:97,
        B:98,
        C:99,
        D:100,
        E:101,
        F:102,
        G:103,
        H:104,
        I:105,
        J:106,
        K:107,
        L:108,
        M:109,
        N:110,
        O:111,
        P:112,
        Q:113,
        R:114,
        S:115,
        T:116,
        U:117,
        V:118,
        W:119,
        X:120,
        Y:121,
        Z:122,
        DELETE:127,
        CAPSLOCK:1073741881,
        F1:1073741882,
        F2:1073741883,
        F3:1073741884,
        F4:1073741885,
        F5:1073741886,
        F6:1073741887,
        F7:1073741888,
        F8:1073741889,
        F9:1073741890,
        F10:1073741891,
        F11:1073741892,
        F12:1073741893,
        PRINTSCREEN:1073741894,
        SCROLLLOCK:1073741895,
        PAUSE:1073741896,
        INSERT:1073741897,
        HOME:1073741898,
        PAGEUP:1073741899,
        END:1073741901,
        PAGEDOWN:1073741902,
        RIGHT:1073741903,
        LEFT:1073741904,
        DOWN:1073741905,
        UP:1073741906,
        NUMLOCKCLEAR:1073741907,
        KP_DIVIDE:1073741908,
        KP_MULTIPLY:1073741909,
        KP_MINUS:1073741910,
        KP_PLUS:1073741911,
        KP_ENTER:1073741912,
        KP_1:1073741913,
        KP_2:1073741914,
        KP_3:1073741915,
        KP_4:1073741916,
        KP_5:1073741917,
        KP_6:1073741918,
        KP_7:1073741919,
        KP_8:1073741920,
        KP_9:1073741921,
        KP_0:1073741922,
        KP_PERIOD:1073741923,
        APPLICATION:1073741925,
        POWER:1073741926,
        KP_EQUALS:1073741927,
        F13:1073741928,
        F14:1073741929,
        F15:1073741930,
        F16:1073741931,
        F17:1073741932,
        F18:1073741933,
        F19:1073741934,
        F20:1073741935,
        F21:1073741936,
        F22:1073741937,
        F23:1073741938,
        F24:1073741939,
        EXECUTE:1073741940,
        HELP:1073741941,
        MENU:1073741942,
        SELECT:1073741943,
        STOP:1073741944,
        AGAIN:1073741945,
        UNDO:1073741946,
        CUT:1073741947,
        COPY:1073741948,
        PASTE:1073741949,
        FIND:1073741950,
        MUTE:1073741951,
        VOLUMEUP:1073741952,
        VOLUMEDOWN:1073741953,
        KP_COMMA:1073741957,
        KP_EQUALSAS400:1073741958,
        ALTERASE:1073741977,
        SYSREQ:1073741978,
        CANCEL:1073741979,
        CLEAR:1073741980,
        PRIOR:1073741981,
        RETURN2:1073741982,
        SEPARATOR:1073741983,
        OUT:1073741984,
        OPER:1073741985,
        CLEARAGAIN:1073741986,
        CRSEL:1073741987,
        EXSEL:1073741988,
        KP_00:1073742000,
        KP_000:1073742001,
        THOUSANDSEPARATOR:1073742002,
        DECIMALSEPARATOR:1073742003,
        CURRENCYUNIT:1073742004,
        CURRENCYSUBUNIT:1073742005,
        KP_LEFTPAREN:1073742006,
        KP_RIGHTPAREN:1073742007,
        KP_LEFTBRACE:1073742008,
        KP_RIGHTBRACE:1073742009,
        KP_TAB:1073742010,
        KP_BACKSPACE:1073742011,
        KP_A:1073742012,
        KP_B:1073742013,
        KP_C:1073742014,
        KP_D:1073742015,
        KP_E:1073742016,
        KP_F:1073742017,
        KP_XOR:1073742018,
        KP_POWER:1073742019,
        KP_PERCENT:1073742020,
        KP_LESS:1073742021,
        KP_GREATER:1073742022,
        KP_AMPERSAND:1073742023,
        KP_DBLAMPERSAND:1073742024,
        KP_VERTICALBAR:1073742025,
        KP_DBLVERTICALBAR:1073742026,
        KP_COLON:1073742027,
        KP_HASH:1073742028,
        KP_SPACE:1073742029,
        KP_AT:1073742030,
        KP_EXCLAM:1073742031,
        KP_MEMSTORE:1073742032,
        KP_MEMRECALL:1073742033,
        KP_MEMCLEAR:1073742034,
        KP_MEMADD:1073742035,
        KP_MEMSUBTRACT:1073742036,
        KP_MEMMULTIPLY:1073742037,
        KP_MEMDIVIDE:1073742038,
        KP_PLUSMINUS:1073742039,
        KP_CLEAR:1073742040,
        KP_CLEARENTRY:1073742041,
        KP_BINARY:1073742042,
        KP_OCTAL:1073742043,
        KP_DECIMAL:1073742044,
        KP_HEXADECIMAL:1073742045,
        LCTRL:1073742048,
        LSHIFT:1073742049,
        LALT:1073742050,
        LGUI:1073742051,
        RCTRL:1073742052,
        RSHIFT:1073742053,
        RALT:1073742054,
        RGUI:1073742055,
        MODE:1073742081,
        AUDIONEXT:1073742082,
        AUDIOPREV:1073742083,
        AUDIOSTOP:1073742084,
        AUDIOPLAY:1073742085,
        AUDIOMUTE:1073742086,
        MEDIASELECT:1073742087,
        WWW:1073742088,
        MAIL:1073742089,
        CALCULATOR:1073742090,
        COMPUTER:1073742091,
        AC_SEARCH:1073742092,
        AC_HOME:1073742093,
        AC_BACK:1073742094,
        AC_FORWARD:1073742095,
        AC_STOP:1073742096,
        AC_REFRESH:1073742097,
        AC_BOOKMARKS:1073742098,
        BRIGHTNESSDOWN:1073742099,
        BRIGHTNESSUP:1073742100,
        DISPLAYSWITCH:1073742101,
        KBDILLUMTOGGLE:1073742102,
        KBDILLUMDOWN:1073742103,
        KBDILLUMUP:1073742104,
        EJECT:1073742105,
        SLEEP:1073742106
        

    };


    return class(
        name: 'SES.Input',
        
        
        
        define:::(this) {
            @:ACTIONS =  {
                ADD : 0,
                REMOVE : 1
            };
            this.interface = {
                DEVICES : {
                    get ::<- DEVICES
                },
                KEYS : {
                    get::<- KEYS
                },
                EVENTS: {
                    get::<- EVENTS
                },
                POINTER_BUTTONS : {
                    get::<- POINTER_BUTTONS
                },
                
                
                // motion events:
                // - event (EVENTS.POINTER_MOTION)
                // - x
                // - y
                
                // keydown events:
                // - event (EVENTS.KEY_DOWN),
                // - key (KEYS)
                addCallback ::(
                    device => Number,
                    callback => Function
                ){
                    if (device < 0 || device > DEVICES.GAMEPAD3) error(detail:'Unrecognized device');
                    return ses_native__input_attrib(a:ACTIONS.ADD, b:device, c:callback); 
                },
                
                
                removeCallback ::(id => Number, device => Number) {
                    ses_native__input_attrib(a:ACTIONS.REMOVE, b:device, c:id);
                }
            };
        }
    ).new();
};



@:Audio = ::<= {
    @:ACTIONS = {
        PLAY: 0,
        HALT: 1,
        VOLUME: 2,
        PANNING: 3
    };


    return class(
        name: 'SES.Audio',
        define:::(this) {
            this.interface = {
                // play a sample to a channel.
                // the channels audio is halted
                play::(
                    sample  => Number,
                    channel => Number,  // 0 - 31
                    loop    => Boolean
                    
                ) {
                    ses_native__audio_attrib(a:ACTIONS.PLAY, b:sample, c:channel, d:loop);
                },
                
                // stops audio from a channel
                halt::(
                    channel => Number
                ) {
                    ses_native__audio_attrib(a:ACTIONS.HALT, b:channel);
                },
                
                // sets the volume for a channel
                setVolume :: (
                    channel => Number,
                    amount => Number
                ) {
                    ses_native__audio_attrib(a:ACTIONS.VOLUME, b:channel, c:amount);
                },
                
                // sets the panning for the channel
                setPanning :: (
                    channel => Number,
                    amount => Number   
                ) {
                    ses_native__audio_attrib(a:ACTIONS.PANNING, b:channel, c:amount);
                }
            };
        }
    ).new();
};


// backgrounds are specifically
// sets of tiles collated together.
//
// Tiles above id 0x40000 correspond to 
// background tiles. Backgrounds ALWAYS read 
// the same tiles, so the user will work with 
// backgrounds by populating the tiles 
// corresponding to the expected IDs.
//
// IDs are ordered from topleft to bottomright
// in rows, where each row is 32 tiles with 16 rows 
// per background.
//
// NOTE: background tiles CANNOT be used as 
// sprites.
@:Background = ::<= {
    
    @:ATTRIBS = {
        ENABLE:    0,
        POSITIONX :1,
        POSITIONY :2,
        LAYER     :3,
        EFFECT    :4,
        PALETTE   :5
    };
    
    
    @:EFFECTS = {
        COLOR:             0,
        MASK:              1, // only stencil information 
        MASK_AND_COLOR:    2, // stencil and color
        COLOR_ON_MASK:     3, // color only on stencil areas 
        COLOR_AROUND_MASK: 4, // color only where there is no stencil
        BLEND:             5, // adds color information, giving a transparent ghost image effect 
        BLEND_ON_MASK:     6, // blend, but only masks,
        BLEIND_AROUND_MASK:7, // blend, but only in non-mask areas
    };
    
    return class(
        name: 'SES.Background',
        statics : {
            WIDTH_TILES:32,
            HEIGHT_TILES:16
        },
        define:::(this) {
            this.interface = {
                set::(
                    index => Number,
                    show,
                    x,
                    y,
                    layer,
                    effect,
                    palette
                ) {
                    if (show != empty) 
                        ses_native__bg_attrib(a:index, b:ATTRIBS.ENABLE, c:if((show => Boolean) == true) 1 else 0);
 
                    if (x != empty)
                        ses_native__bg_attrib(a:index, b:ATTRIBS.POSITIONX, c:x=>Number);

                    if (y != empty)
                        ses_native__bg_attrib(a:index, b:ATTRIBS.POSITIONY, c:y=>Number);


                    if (layer != empty)
                        ses_native__bg_attrib(a:index, b:ATTRIBS.LAYER, c:layer=>Number);            

                    if (effect != empty)                        
                        ses_native__bg_attrib(a:index, b:ATTRIBS.EFFECT, c:effect=>Number);                        

                    if (palette != empty)
                        ses_native__bg_attrib(a:index, b:ATTRIBS.PALETTE, c:palette=>Number);

                },
                
                EFFECTS: {
                    get::<-EFFECTS
                }
            };
        }
    ).new();
};



// Sprites are single-tile objects that 
// have specific attributes.
//
// NOTE: background tiles (tiles above ID 0x40000-1) CANNOT be used as 
// sprites.
@:Sprite = ::<= {
    @spriteIDPool = [];
    @spriteID = 0;
    
    @:ATTRIBS = {
        ENABLE:    0,
        ROTATION:  1,
        SCALEX:    2,
        SCALEY:    3,
        POSITIONX :4,
        POSITIONY :5,
        CENTERX   :6,
        CENTERY   :7,
        LAYER     :8,
        TILEINDEX:9,
        EFFECT    :10,
        PALETTE   :11,
    };
    
    
    
    @:EFFECTS = {
        COLOR:             0,
        MASK:              1, // only stencil information 
        MASK_AND_COLOR:    2, // stencil and color
        COLOR_ON_MASK:     3, // color only on stencil areas 
        COLOR_AROUND_MASK: 4, // color only where there is no stencil
        BLEND:             5, // adds color information, giving a transparent ghost image effect 
        BLEND_ON_MASK:     6, // blend, but only masks,
        BLEND_AROUND_MASK: 7, // blend, but only in non-mask areas
    };

    
    return class(
        name: 'SES.Sprite',
        define:::(this) {
            this.interface = {
                set::(
                    index => Number,
                    show,
                    tile,
                    x,
                    y,
                    layer,
                    effect,
                    palette
                ) {
                    @commands = [];
                    if (show != empty) commands     = [...commands, ATTRIBS.ENABLE, if((show => Boolean) == true) 1 else 0];
                    if (tile != empty) commands     = [...commands, ATTRIBS.TILEINDEX, tile=>Number];
                    if (x != empty) commands        = [...commands, ATTRIBS.POSITIONX, x=>Number];
                    if (y != empty) commands        = [...commands, ATTRIBS.POSITIONY, y=>Number];
                    if (layer != empty) commands    = [...commands, ATTRIBS.LAYER, layer=>Number];            
                    if (effect != empty) commands   = [...commands, ATTRIBS.EFFECT, effect=>Number];                        
                    if (palette != empty) commands  = [...commands, ATTRIBS.PALETTE, palette=>Number];

                    ses_native__sprite_attrib(a:index, b:commands);

                },
                
                transform::(
                    index => Number,
                    scaleX,
                    scaleY,
                    centerX,
                    centerY,
                    rotation
                               
                ) {
                    @commands = [];

                    if (scaleX != empty) commands   = [...commands, ATTRIBS.SCALEX, scaleX=>Number];
                    if (scaleY != empty) commands   = [...commands, ATTRIBS.SCALEY, scaleY=>Number];
                    if (rotation != empty) commands = [...commands, ATTRIBS.ROTATION, rotation=>Number];
                    if (centerX != empty) commands  = [...commands, ATTRIBS.CENTERX, centerX=>Number];
                    if (centerY != empty) commands  = [...commands, ATTRIBS.CENTERY, centerY=>Number];

                    ses_native__sprite_attrib(a:index, b:commands);
                
                },
             
                
                EFFECTS: {
                    get::<-EFFECTS
                }
            };
        }
    ).new();
};



@:Oscillator = ::<= {
    @:ATTRIBS = {
        ENABLE:    0,
        PERIODMS:  1,
        ONCYCLE:   2,
        GET:       3
    };
    
    return class(
        name: 'SES.Oscillator',
        define:::(this) {
            this.interface = {
                set ::(
                    index => Number,
                    enable,
                    periodMS,
                    onCycle
                ) {
                    @commands = [];

                    if (enable != empty) commands   = [...commands, ATTRIBS.ENABLE, enable=>Boolean];
                    if (periodMS != empty) commands   = [...commands, ATTRIBS.PERIODMS, periodMS=>Number];
                    if (onCycle != empty) commands = [...commands, ATTRIBS.ONCYCLE, onCycle=>Function];

                    ses_native__oscillator_attrib(a:index, b:commands);
                                    
                },
                
                
                get ::(index => Number) {
                    return ses_native__oscillator_attrib(a:index, b:[ATTRIBS.GET, 0]);
                }
            
            };
        }
    ).new();
};

@:SES = class(
    name: 'SES',
    
    define:::(this) {
        @:ATTRIBS = {
            UPDATERATE:  0,
            UPDATEFUNC:  1,
            RESOLUTION:  2,
            CLIPBOARDGET:3,
            CLIPBOARDSET:4
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
            Oscillator: {get ::<- Oscillator},

            RESOLUTION : RESOLUTION,

            resolution : {
                set ::(value => Number) {
                    if (value > RESOLUTION.MD || value < 0) error(detail: 'Invalid resolution type.');
                    resolution = value;
                },
                get ::<- resolution
            },
            
            resolutionWidth : {
                get ::<- match(resolution) {
                  (RESOLUTION.NES): 256,
                  (RESOLUTION.GBA): 240,
                  (RESOLUTION.GBC): 160,
                  (RESOLUTION.MD):  320
                }
            },
            
            resolutionHeight : {
                get ::<- match(resolution) {
                  (RESOLUTION.NES):240,
                  (RESOLUTION.GBA):160,
                  (RESOLUTION.GBC):144,
                  (RESOLUTION.MD): 224
                }
            },
            
            
            clipboard : {
                get ::<- ses_native__engine_attrib(a:ATTRIBS.CLIPBOARDGET),
                set ::(value => String) <- ses_native__engine_attrib(a:ATTRIBS.CLIPBOARDSET, b:value)
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
).new();


@:offset = 0;
Tile.set(
    index: offset,
    data: [
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
    ]
);

Tile.set(
    index: ' '->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    ]
);

Tile.set(
    index: '\t'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    ]
);

Tile.set(
    index: '\n'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    ]
);                

Tile.set(
    index: 'A'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 4, 0, 4, 0, 0, 0, 0,
        0, 4, 0, 4, 0, 0, 0, 0,
        0, 4, 4, 4, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0
    ]
);


Tile.set(
    index: 'B'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 4, 4, 4, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 4, 4, 4, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 4, 4, 4, 0, 0, 0, 0
    ]
);

Tile.set(
    index: 'C'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 4, 4, 4, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        0, 4, 4, 4, 0, 0, 0, 0
    ]
);


Tile.set(
    index: 'D'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 4, 4, 4, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 4, 4, 4, 0, 0, 0, 0
    ]
);

Tile.set(
    index: 'E'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 4, 4, 4, 4, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 4, 4, 4, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 4, 4, 4, 4, 0, 0, 0
    ]
);

Tile.set(
    index: 'F'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 4, 4, 4, 4, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 4, 4, 4, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0
    ]
); 


Tile.set(
    index: 'G'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 4, 4, 4, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 4, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        0, 4, 4, 4, 4, 0, 0, 0
    ]
);    

Tile.set(
    index: 'H'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 4, 4, 4, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0
    ]
);

Tile.set(
    index: 'I'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 4, 4, 4, 4, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        4, 4, 4, 4, 4, 0, 0, 0
    ]
);    


Tile.set(
    index: 'J'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 4, 4, 4, 0, 0, 0,
        0, 0, 0, 0, 4, 0, 0, 0,
        0, 0, 0, 0, 4, 0, 0, 0,
        0, 0, 0, 0, 4, 0, 0, 0,
        0, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        0, 4, 4, 4, 0, 0, 0, 0
    ]
);


Tile.set(
    index: 'K'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 4, 0, 0, 0, 0,
        4, 0, 4, 0, 0, 0, 0, 0,
        4, 4, 0, 0, 0, 0, 0, 0,
        4, 0, 4, 0, 0, 0, 0, 0,
        4, 0, 0, 4, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0
    ]
);    

Tile.set(
    index: 'L'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 4, 4, 4, 4, 0, 0, 0
    ]
);

Tile.set(
    index: 'M'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 4, 0, 4, 4, 0, 0, 0,
        4, 0, 4, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0
    ]
);

Tile.set(
    index: 'N'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 4, 0, 0, 4, 0, 0, 0,
        4, 0, 4, 0, 4, 0, 0, 0,
        4, 0, 0, 4, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0
    ]
);

Tile.set(
    index: 'O'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 4, 4, 4, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        0, 4, 4, 4, 0, 0, 0, 0
    ]
);

Tile.set(
    index: 'P'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 4, 4, 4, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 4, 4, 4, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0
    ]
);

Tile.set(
    index: 'Q'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 4, 4, 4, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 4, 0, 4, 0, 0, 0,
        4, 0, 0, 4, 0, 0, 0, 0,
        0, 4, 4, 0, 4, 0, 0, 0
    ]
);

Tile.set(
    index: 'R'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 4, 4, 4, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 4, 4, 4, 0, 0, 0, 0,
        4, 0, 4, 0, 0, 0, 0, 0,
        4, 0, 0, 4, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0
    ]
);

Tile.set(
    index: 'S'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 4, 4, 4, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        0, 4, 4, 4, 0, 0, 0, 0,
        0, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        0, 4, 4, 4, 0, 0, 0, 0
    ]
);    

Tile.set(
    index: 'T'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 4, 4, 4, 4, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0
    ]
);    

Tile.set(
    index: 'U'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        0, 4, 4, 4, 0, 0, 0, 0
    ]
);

Tile.set(
    index: 'V'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        0, 4, 0, 4, 0, 0, 0, 0,
        0, 4, 0, 4, 0, 0, 0, 0,
        0, 4, 0, 4, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0
    ]
);

Tile.set(
    index: 'W'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 4, 0, 4, 0, 0, 0,
        4, 0, 4, 0, 4, 0, 0, 0,
        0, 4, 0, 4, 0, 0, 0, 0,
        0, 4, 0, 4, 0, 0, 0, 0,
        0, 4, 0, 4, 0, 0, 0, 0
    ]
);

Tile.set(
    index: 'X'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        0, 4, 0, 4, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 4, 0, 4, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0
    ]
);    

Tile.set(
    index: 'Y'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        0, 4, 0, 4, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0
    ]
);    

Tile.set(
    index: 'Z'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 4, 4, 4, 4, 0, 0, 0,
        0, 0, 0, 0, 4, 0, 0, 0,
        0, 0, 0, 4, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 4, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 4, 4, 4, 4, 0, 0, 0
    ]
);






Tile.set(
    index: 'a'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 4, 4, 4, 0, 0, 0, 0,
        0, 0, 0, 0, 4, 0, 0, 0,
        0, 4, 4, 4, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        0, 4, 4, 4, 4, 0, 0, 0
    ]
);

Tile.set(
    index: 'b'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 4, 4, 0, 0, 0, 0,
        4, 4, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 4, 4, 4, 0, 0, 0, 0
    ]
);

Tile.set(
    index: 'c'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 4, 4, 4, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        0, 4, 4, 4, 0, 0, 0, 0
    ]
);    

Tile.set(
    index: 'd'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 4, 0, 0, 0,
        0, 0, 0, 0, 4, 0, 0, 0,
        0, 4, 4, 0, 4, 0, 0, 0,
        4, 0, 0, 4, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        0, 4, 4, 4, 4, 0, 0, 0
    ]
);

Tile.set(
    index: 'e'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 4, 4, 4, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 4, 4, 4, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        0, 4, 4, 4, 4, 0, 0, 0
    ]
);


Tile.set(
    index: 'f'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 4, 4, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 4, 4, 4, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0
    ]
);

Tile.set(
    index: 'g'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 4, 4, 4, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 4, 4, 0, 0, 0,
        0, 4, 4, 0, 4, 0, 0, 0,
        0, 0, 0, 0, 4, 0, 0, 0,
        4, 4, 4, 4, 0, 0, 0, 0
    ]
);

Tile.set(
    index: 'h'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 4, 4, 0, 0, 0, 0,
        4, 4, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0
    ]
);


Tile.set(
    index: 'i'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 4, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        4, 4, 4, 4, 4, 0, 0, 0
    ]
);

Tile.set(
    index: 'j'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 4, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 4, 4, 4, 0, 0, 0,
        0, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        0, 4, 4, 4, 0, 0, 0, 0
    ]
);    

Tile.set(
    index: 'k'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 4, 0, 0, 0, 0,
        4, 4, 4, 0, 0, 0, 0, 0,
        4, 0, 0, 4, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0
    ]
);

Tile.set(
    index: 'l'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 4, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 0, 4, 4, 0, 0, 0
    ]
);

Tile.set(
    index: 'm'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 4, 0, 0, 0, 0, 0,
        4, 4, 4, 4, 0, 0, 0, 0,
        4, 0, 4, 0, 4, 0, 0, 0,
        4, 0, 4, 0, 4, 0, 0, 0,
        4, 0, 4, 0, 4, 0, 0, 0
    ]
);

Tile.set(
    index: 'n'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 4, 4, 0, 0, 0, 0,
        4, 4, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0
    ]
);

Tile.set(
    index: 'o'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 4, 4, 4, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        0, 4, 4, 4, 0, 0, 0, 0
    ]
);

Tile.set(
    index: 'p'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 4, 4, 0, 0, 0, 0,
        4, 4, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 4, 4, 4, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0
    ]
);

Tile.set(
    index: 'q'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 4, 4, 4, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 4, 4, 0, 0, 0,
        0, 4, 4, 0, 4, 0, 0, 0,
        0, 0, 0, 0, 4, 0, 0, 0,
        0, 0, 0, 0, 4, 0, 0, 0
    ]
);    

Tile.set(
    index: 'r'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 4, 4, 0, 0, 0, 0,
        4, 4, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0
    ]
);

Tile.set(
    index: 's'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 4, 4, 4, 4, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        0, 4, 4, 4, 0, 0, 0, 0,
        0, 0, 0, 0, 4, 0, 0, 0,
        4, 4, 4, 4, 0, 0, 0, 0
    ]
);

Tile.set(
    index: 't'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        4, 4, 4, 4, 4, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 0, 4, 4, 0, 0, 0
    ]
);

Tile.set(
    index: 'u'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 4, 4, 0, 0, 0,
        0, 4, 4, 0, 4, 0, 0, 0
    ]
);

Tile.set(
    index: 'v'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        0, 4, 0, 4, 0, 0, 0, 0,
        0, 4, 0, 4, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0
    ]
);


Tile.set(
    index: 'w'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 4, 0, 4, 0, 0, 0,
        4, 0, 4, 0, 4, 0, 0, 0,
        0, 4, 0, 4, 0, 0, 0, 0,
        0, 4, 0, 4, 0, 0, 0, 0
    ]
);

Tile.set(
    index: 'x'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        0, 4, 0, 4, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 4, 0, 4, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0
    ]
);    

Tile.set(
    index: 'y'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 4, 4, 0, 0, 0,
        0, 4, 4, 0, 4, 0, 0, 0,
        0, 0, 0, 0, 4, 0, 0, 0,
        4, 4, 4, 4, 0, 0, 0, 0
    ]
);    

Tile.set(
    index: 'z'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 4, 4, 4, 4, 0, 0, 0,
        0, 0, 0, 4, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 4, 0, 0, 0, 0, 0, 0,
        4, 4, 4, 4, 4, 0, 0, 0
    ]
);

Tile.set(
    index: '0'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 4, 4, 4, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 4, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        0, 4, 4, 4, 0, 0, 0, 0
    ]
);

Tile.set(
    index: '1'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 4, 4, 0, 0, 0, 0, 0,
        4, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        4, 4, 4, 4, 4, 0, 0, 0
    ]
);

Tile.set(
    index: '2'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 4, 4, 4, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        0, 0, 0, 0, 4, 0, 0, 0,
        0, 0, 0, 4, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 4, 0, 0, 0, 0, 0, 0,
        4, 4, 4, 4, 4, 0, 0, 0
    ]
);    

Tile.set(
    index: '3'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 4, 4, 4, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        0, 0, 0, 0, 4, 0, 0, 0,
        0, 0, 4, 4, 0, 0, 0, 0,
        0, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        0, 4, 4, 4, 0, 0, 0, 0
    ]
);


Tile.set(
    index: '4'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 4, 0, 0, 0, 0,
        0, 0, 4, 4, 0, 0, 0, 0,
        0, 4, 0, 4, 0, 0, 0, 0,
        4, 4, 4, 4, 4, 0, 0, 0,
        0, 0, 0, 4, 0, 0, 0, 0,
        0, 0, 0, 4, 0, 0, 0, 0,
        0, 0, 0, 4, 0, 0, 0, 0
    ]
);


Tile.set(
    index: '5'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 4, 4, 4, 4, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 4, 4, 4, 0, 0, 0, 0,
        0, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        0, 4, 4, 4, 0, 0, 0, 0
    ]
);

Tile.set(
    index: '6'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 4, 4, 4, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        4, 4, 4, 4, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        0, 4, 4, 4, 0, 0, 0, 0
    ]
);

Tile.set(
    index: '7'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 4, 4, 4, 4, 0, 0, 0,
        0, 0, 0, 0, 4, 0, 0, 0,
        0, 0, 0, 0, 4, 0, 0, 0,
        0, 0, 0, 4, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 4, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0
    ]
);    

Tile.set(
    index: '8'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 4, 4, 4, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        0, 4, 4, 4, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        0, 4, 4, 4, 0, 0, 0, 0
    ]
);


Tile.set(
    index: '9'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 4, 4, 4, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        0, 4, 4, 4, 4, 0, 0, 0,
        0, 0, 0, 0, 4, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        0, 4, 4, 4, 0, 0, 0, 0
    ]
);

Tile.set(
    index: '.'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0
    ]
);                

Tile.set(
    index: ','->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 4, 0, 0, 0, 0, 0, 0
    ]
);

Tile.set(
    index: '"'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 4, 0, 4, 0, 0, 0, 0,
        0, 4, 0, 4, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    ]
);

Tile.set(
    index: '\''->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    ]
);

Tile.set(
    index: '?'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 4, 4, 4, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        0, 0, 0, 0, 4, 0, 0, 0,
        0, 0, 4, 4, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0
    ]
);

Tile.set(
    index: '!'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0
    ]
);

Tile.set(
    index: '@'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 4, 4, 4, 0, 0, 0, 0,
        4, 0, 0, 4, 4, 0, 0, 0,
        4, 0, 4, 4, 4, 0, 0, 0,
        4, 4, 0, 4, 4, 0, 0, 0,
        4, 0, 4, 4, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        0, 4, 4, 4, 4, 0, 0, 0
    ]
);

Tile.set(
    index: '_'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 4, 4, 4, 4, 0, 0, 0
    ]
);

Tile.set(
    index: '*'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        4, 0, 4, 0, 4, 0, 0, 0,
        0, 4, 4, 4, 0, 0, 0, 0,
        4, 0, 4, 0, 4, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    ]
);

Tile.set(
    index: '#'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 4, 0, 4, 0, 0, 0, 0,
        0, 4, 0, 4, 0, 0, 0, 0,
        4, 4, 4, 4, 4, 0, 0, 0,
        0, 4, 0, 4, 0, 0, 0, 0,
        4, 4, 4, 4, 4, 0, 0, 0,
        0, 4, 0, 4, 0, 0, 0, 0,
        0, 4, 0, 4, 0, 0, 0, 0
    ]
);

Tile.set(
    index: '$'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 4, 4, 4, 4, 0, 0, 0,
        4, 0, 4, 0, 0, 0, 0, 0,
        0, 4, 4, 4, 0, 0, 0, 0,
        0, 0, 4, 0, 4, 0, 0, 0,
        4, 4, 4, 4, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0
    ]
);

Tile.set(
    index: '%'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 4, 0, 0, 0, 0, 0, 0,
        4, 4, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 4, 0, 0, 0,
        0, 0, 0, 4, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 4, 0, 4, 4, 0, 0, 0,
        4, 0, 0, 4, 4, 0, 0, 0
    ]
);

Tile.set(
    index: '&'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 4, 0, 0, 0, 0, 0, 0,
        4, 0, 4, 0, 0, 0, 0, 0,
        4, 0, 4, 0, 0, 0, 0, 0,
        0, 4, 0, 0, 0, 0, 0, 0,
        4, 0, 4, 0, 4, 0, 0, 0,
        4, 0, 0, 4, 0, 0, 0, 0,
        0, 4, 4, 0, 4, 0, 0, 0
    ]
);                

Tile.set(
    index: '('->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 4, 0, 0, 0, 0, 0, 0,
        0, 4, 0, 0, 0, 0, 0, 0,
        0, 4, 0, 0, 0, 0, 0, 0,
        0, 4, 0, 0, 0, 0, 0, 0,
        0, 4, 0, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0
    ]
);

Tile.set(
    index: ')'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 0, 4, 0, 0, 0, 0,
        0, 0, 0, 4, 0, 0, 0, 0,
        0, 0, 0, 4, 0, 0, 0, 0,
        0, 0, 0, 4, 0, 0, 0, 0,
        0, 0, 0, 4, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0
    ]
);                

Tile.set(
    index: '+'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        4, 4, 4, 4, 4, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    ]
);

Tile.set(
    index: '-'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 4, 4, 4, 4, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    ]
);

Tile.set(
    index: '/'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 4, 0, 0, 0,
        0, 0, 0, 4, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 4, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    ]
);

Tile.set(
    index: ':'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    ]
);

Tile.set(
    index: ';'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 4, 0, 0, 0, 0, 0, 0
    ]
);

Tile.set(
    index: '<'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 4, 4, 0, 0, 0,
        0, 4, 4, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        0, 4, 4, 0, 0, 0, 0, 0,
        0, 0, 0, 4, 4, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    ]
);

Tile.set(
    index: '='->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 4, 4, 4, 4, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 4, 4, 4, 4, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    ]
);
Tile.set(
    index: '>'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 4, 0, 0, 0, 0, 0, 0,
        0, 0, 4, 4, 0, 0, 0, 0,
        0, 0, 0, 0, 4, 0, 0, 0,
        0, 0, 4, 4, 0, 0, 0, 0,
        4, 4, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    ]
);

Tile.set(
    index: '['->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 4, 4, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 4, 0, 0, 0, 0
    ]
);

Tile.set(
    index: '\\'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        0, 4, 0, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 0, 4, 0, 0, 0, 0,
        0, 0, 0, 0, 4, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    ]
);

Tile.set(
    index: ']'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 4, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 4, 4, 0, 0, 0, 0, 0
    ]
);

Tile.set(
    index: '^'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 4, 0, 4, 0, 0, 0, 0,
        4, 0, 0, 0, 4, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    ]
);

Tile.set(
    index: '`'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 4, 0, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    ]
);                

Tile.set(
    index: '{'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 4, 4, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 4, 0, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 4, 0, 0, 0, 0
    ]
);

Tile.set(
    index: '|'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0
    ]
);

Tile.set(
    index: '}'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 4, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 0, 4, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 4, 0, 0, 0, 0, 0,
        0, 4, 4, 0, 0, 0, 0, 0
    ]
);

Tile.set(
    index: '~'->charCodeAt(index:0)+offset,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 4, 0, 0, 0, 0, 0, 0,
        4, 0, 4, 0, 4, 0, 0, 0,
        0, 0, 0, 4, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    ]
);


lockTiles = true;

return SES;

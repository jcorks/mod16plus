@:class = import(module:'Matte.Core.Class');



@:mod16_native__get_context_cartridge_id = getExternalFunction(name:"mod16_native__get_context_cartridge_id");
@:mod16_native__has_boot_context = getExternalFunction(name:"mod16_native__has_boot_context");
@:mod16_native__get_source = getExternalFunction(name:"mod16_native__get_source");
@:mod16_native__get_sub_cartridge_main = getExternalFunction(name:"mod16_native__get_sub_cartridge_main");


@:mod16_native__sprite_attrib = getExternalFunction(name:"mod16_native__sprite_attrib");
@:mod16_native__palette_attrib = getExternalFunction(name:"mod16_native__palette_attrib");
@:mod16_native__tile_attrib = getExternalFunction(name:"mod16_native__tile_attrib");
@:mod16_native__audio_attrib = getExternalFunction(name:"mod16_native__audio_attrib");
@:mod16_native__bg_attrib = getExternalFunction(name:"mod16_native__bg_attrib");
@:mod16_native__oscillator_attrib = getExternalFunction(name:"mod16_native__oscillator_attrib");
// query functions are necessary because they are (can be) pre-populated by the ROM.
@:mod16_native__palette_query = getExternalFunction(name:"mod16_native__palette_query");
@:mod16_native__tile_query = getExternalFunction(name:"mod16_native__tile_query");




@:mod16_native__input_attrib = getExternalFunction(name:"mod16_native__input_attrib");
@:mod16_native__engine_attrib = getExternalFunction(name:"mod16_native__engine_attrib");



@:mod16_native__debug_context_is_allowed = getExternalFunction(name:"mod16_native__debug_context_is_allowed");
@:mod16_native__debug_context_enter = getExternalFunction(name:"mod16_native__debug_context_enter");
@:mod16_native__debug_context_query = getExternalFunction(name:"mod16_native__debug_context_query");
@:mod16_native__debug_context_bind = getExternalFunction(name:"mod16_native__debug_context_bind");


@:mod16_native__vertices_set_count = getExternalFunction(name:"mod16_native__vertices_set_count");
@:mod16_native__vertices_set_shape = getExternalFunction(name:"mod16_native__vertices_set_shape");
@:mod16_native__vertices_set_transform = getExternalFunction(name:"mod16_native__vertices_set_transform");
@:mod16_native__vertices_set_effect = getExternalFunction(name:"mod16_native__vertices_set_effect");
@:mod16_native__vertices_set_layer = getExternalFunction(name:"mod16_native__vertices_set_layer");
@:mod16_native__vertices_set_palette = getExternalFunction(name:"mod16_native__vertices_set_palette");
@:mod16_native__vertices_set_textured = getExternalFunction(name:"mod16_native__vertices_set_textured");
@:mod16_native__vertices_set = getExternalFunction(name:"mod16_native__vertices_set");                                   
@:mod16_native__vertices_get = getExternalFunction(name:"mod16_native__vertices_get");


@:package_native__is_packaging_allowed  = getExternalFunction(name:"package_native__is_packaging_allowed");
@:package_native__save_source  = getExternalFunction(name:"package_native__save_source");
@:package_native__make_project = getExternalFunction(name:"package_native__make_project");
@:package_native__open_source  = getExternalFunction(name:"package_native__open_source");
@:package_native__list_projects= getExternalFunction(name:"package_native__list_projects");

                





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
        name: 'MOD16.Palette',
        
        
        define:::(this) {
        
            @cartID_;
            this.constructor = ::(cartID) {
                cartID_ = cartID;
                return this;
            };
        
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

                    mod16_native__palette_attrib(a:cartID_, b:index, c:COLORS.BACK,     d:colorBack[0],     e:colorBack[1],     f:colorBack[2]);
                    mod16_native__palette_attrib(a:cartID_, b:index, c:COLORS.MIDBACK,  d:colorMidBack[0],  e:colorMidBack[1],  f:colorMidBack[2]);
                    mod16_native__palette_attrib(a:cartID_, b:index, c:COLORS.MIDFRONT, d:colorMidFront[0], e:colorMidFront[1], f:colorMidFront[2]);
                    mod16_native__palette_attrib(a:cartID_, b:index, c:COLORS.FRONT,    d:colorFront[0],    e:colorFront[1],    f:colorFront[2]);
                        
                },
                
                
                get::(
                    index => Number
                ) {
                    return [
                        [
                            mod16_native__palette_query(a:cartID_, b:index, c:COLORS.BACK, d:0),
                            mod16_native__palette_query(a:cartID_, b:index, c:COLORS.BACK, d:1),
                            mod16_native__palette_query(a:cartID_, b:index, c:COLORS.BACK, d:2)
                        ],
                        
                        [
                            mod16_native__palette_query(a:cartID_, b:index, c:COLORS.MIDBACK, d:0),
                            mod16_native__palette_query(a:cartID_, b:index, c:COLORS.MIDBACK, d:1),
                            mod16_native__palette_query(a:cartID_, b:index, c:COLORS.MIDBACK, d:2)
                        ],
                        
                        [
                            mod16_native__palette_query(a:cartID_, b:index, c:COLORS.MIDFRONT, d:0),
                            mod16_native__palette_query(a:cartID_, b:index, c:COLORS.MIDFRONT, d:1),
                            mod16_native__palette_query(a:cartID_, b:index, c:COLORS.MIDFRONT, d:2)
                        ],
                    
                        [
                            mod16_native__palette_query(a:cartID_, b:index, c:COLORS.FRONT, d:0),
                            mod16_native__palette_query(a:cartID_, b:index, c:COLORS.FRONT, d:1),
                            mod16_native__palette_query(a:cartID_, b:index, c:COLORS.FRONT, d:2)
                        ]
                    ];
                }
            };        
        }
    );
};


// preset tiles are loaded from the rom
@:Tile = ::<= {
    @:ATTRIBS = {
        SET        : 0,
        COPY       : 1
    };

    return class(
        name: 'MOD16.Tile',
        
        define:::(this) {
            @cartID_;
            
            this.constructor = ::(cartID) {
                cartID_ = cartID;
                return this;
            };
        
        
            this.interface = {
            
                // data is a plain array of numbers, 0 - 4
                set ::(index => Number, data => Object) {                    
                    mod16_native__tile_attrib(
                        a:cartID_, 
                        b:index,
                        c:ATTRIBS.SET,
                        d:data
                    );
                },
                
                
                get ::(index => Number) {
                    return mod16_native__tile_query(a:cartID_, b:index);
                },
                
                copy ::(to => Number, from => Number) {
                    mod16_native__tile_attrib(a:cartID_, b:from, c:ATTRIBS.COPY, d:to);
                }

            };











        }
    );
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
        name: 'MOD16.Input',
        
        
        
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
                    return mod16_native__input_attrib(a:ACTIONS.ADD, b:device, c:callback); 
                },
                
                
                removeCallback ::(id => Number, device => Number) {
                    mod16_native__input_attrib(a:ACTIONS.REMOVE, b:device, c:id);
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
        name: 'MOD16.Audio',
        define:::(this) {
            this.interface = {
                
                // stops audio from a channel
                halt::(
                    channel => Number
                ) {
                    mod16_native__audio_attrib(a:ACTIONS.HALT, b:channel);
                },
                
                // sets the volume for a channel
                setVolume :: (
                    channel => Number,
                    amount => Number
                ) {
                    mod16_native__audio_attrib(a:ACTIONS.VOLUME, b:channel, c:amount);
                },
                
                // sets the panning for the channel
                setPanning :: (
                    channel => Number,
                    amount => Number   
                ) {
                    mod16_native__audio_attrib(a:ACTIONS.PANNING, b:channel, c:amount);
                }
            };
        }
    ).new();
};

@:AudioStore = class(
    name: 'MOD16.Audio',
    define:::(this) {
        @cartID_;
        this.constructor = ::(cartID) {
            cartID_ = cartID;
            return this;
        };
    
        this.interface = {
            // play a sample to a channel.
            // the channels audio is halted
            play::(
                sample  => Number,
                channel => Number,  // 0 - 31
                loop    => Boolean
                
            ) {
                mod16_native__audio_attrib(a:cartID_, b:Audio.ACTIONS.PLAY, c:sample, d:channel, e:loop);
            }
        };
    }
);


// backgrounds are specifically
// sets of tiles collated together.
//
// Backgrounds ALWAYS read 
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
        name: 'MOD16.Background',
        statics : {
            WIDTH_TILES:32,
            HEIGHT_TILES:16
        },
        define:::(this) {
            @cartID_;
            this.constructor = ::(cartID) {
                cartID_ = cartID;
                return this;
            };
        
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
                        mod16_native__bg_attrib(a:cartID_, b:index, c:ATTRIBS.ENABLE, d:if((show => Boolean) == true) 1 else 0);
 
                    if (x != empty)
                        mod16_native__bg_attrib(a:cartID_, b:index, c:ATTRIBS.POSITIONX, d:x=>Number);

                    if (y != empty)
                        mod16_native__bg_attrib(a:cartID_, b:index, c:ATTRIBS.POSITIONY, d:y=>Number);


                    if (layer != empty)
                        mod16_native__bg_attrib(a:cartID_, b:index, c:ATTRIBS.LAYER, d:layer=>Number);            

                    if (effect != empty)                        
                        mod16_native__bg_attrib(a:cartID_, b:index, c:ATTRIBS.EFFECT, d:effect=>Number);                        

                    if (palette != empty)
                        mod16_native__bg_attrib(a:cartID_, b:index, c:ATTRIBS.PALETTE, d:palette=>Number);

                },
                
                EFFECTS: {
                    get::<-EFFECTS
                }
            };
        }
    );
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
        name: 'MOD16.Sprite',
        define:::(this) {
            @cartID_;
            this.constructor = ::(cartID) {
                cartID_ = cartID;
                return this;
            };
        
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

                    mod16_native__sprite_attrib(a:cartID_, b:index, c:commands);

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

                    mod16_native__sprite_attrib(a:cartID_, b:index, c:commands);
                
                },
             
                
                EFFECTS: {
                    get::<-EFFECTS
                }
            };
        }
    );
};


@:Vertices = ::<= {
    @:SHAPE = {
        TRIANGLE: 0,
        LINE: 0
    };
    return class(
        name: 'MOD16.Vertices',
        define:::(this) {
            @cartID_;
            this.constructor = ::(cartID) {
                cartID_ = cartID;
                mod16_native__vertices_set_transform(a:cartID_, b:transform);
                return this;
            };
            @palette = 0;
            @count = 0;
            @shape = 0;
            @layer = 0;
            @effect = 0;
            @textured = 0;
            @transform = [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1];
            this.interface = {
                SHAPE : {get::<- SHAPE},
                 
                count : {
                    set ::(value){
                        mod16_native__vertices_set_count(a:cartID_, b:value=>Number);
                        count = value;
                    },

                    get ::{
                        return count;
                    }
                },
                
                shape : {
                    set ::(value) {
                        mod16_native__vertices_set_shape(a:cartID_, b:value=>Number);
                        shape = value;
                    },

                    get :: {
                        return shape;
                    }
                },
                
                transform : {
                    set ::(value => Object) {
                        if (value->keycount != 16)
                            error(detail:'Transform expected to be a 4x4 matrix formatted as a 16-value array');
                        mod16_native__vertices_set_transform(a:cartID_, b:value);
                        transform = [...value];
                    },

                    get :: {
                        return transform;
                    }
                },
                
                effect : {
                    set ::(value => Number) {
                        mod16_native__vertices_set_effect(a:cartID_, b:value);
                        effect = value;
                    },
                    get :: {
                        return effect;
                    }
                },

                layer : {
                    set ::(value => Number) {
                        mod16_native__vertices_set_layer(a:cartID_, b:value);
                        layer = value;
                    },
                    get :: {
                        return layer;
                    }
                },


                palette : {
                    set ::(value => Number) {
                        mod16_native__vertices_set_palette(a:cartID_, b:value);
                        palette = value;
                    },
                    get :: {
                        return palette;
                    }
                },

                layer : {
                    set ::(value => Number) {
                        mod16_native__vertices_set_palette(a:cartID_, b:value);
                        palette = value;
                    },
                    get :: {
                        return palette;
                    }
                },



                textured : {
                    set ::(value => Number) {
                        mod16_native__vertices_set_textured(a:cartID_, b:value);
                        textured = value;
                    },
                    get :: {
                        return textured;
                    }
                },

                
                
                //
                
                //    [x, y, z, r, g, b, u, v, tileID]
                
                setVertex ::(
                    index => Number,
                    data => Object
                ) {
                    when(index >= count)
                        error(detail:'Vertex out of bounds');
                    when(data->keycount != 9)
                        error(detail:'Vertex expects array in format: [x, y, z, r, g, b, u , v, tileID] for all vertices');

                    mod16_native__vertices_set(a:cartID_, b:index, c:data);
                                    
                },
                
                
                getVertex ::(index => Number) {
                    return mod16_native__vertices_get(a:cartID_, b:index);
                },

                vertices : {
                    set:: (
                        value => Object
                    ) {
                        when(value->keycount >= count)
                            error(detail:'Vertex out of bounds');

                        value->foreach(do:::(i, v) {
                            when(v->keycount != 9)
                                error(detail:'Vertex expects array in format: [x, y, z, r, g, b, u , v, tileID] for all vertices');
                            mod16_native__vertices_set(a:cartID_, b:i, c:v);
                        });
                    }   
                }
                
            
            };
        }
    );
};


@:Oscillator = ::<= {
    @:ATTRIBS = {
        ENABLE:    0,
        PERIODMS:  1,
        ONCYCLE:   2,
        GET:       3
    };
    
    return class(
        name: 'MOD16.Oscillator',
        define:::(this) {
            @cartID_;
            this.constructor = ::(cartID) {
                cartID_ = cartID;
                return this;
            };
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

                    mod16_native__oscillator_attrib(a:cartID_, b:index, c:commands);
                                    
                },
                
                
                get ::(index => Number) {
                    return mod16_native__oscillator_attrib(a:cartID_, b:index, c:[ATTRIBS.GET, 0]);
                }
            
            };
        }
    );
};


@:Debug = ::<= {
    @inDebugContext = false;

    @:COLOR_HINT = {
        NORMAL: 0,
        CODE: 1,
        ERROR: 2
    };


    return class(
        define:::(this) {
            this.interface = {
                COLOR_HINT: {
                    get::<-COLOR_HINT
                },
                bind::(
                    onDebugPrint  => Function,
                    onDebugClear  => Function,
                    onDebugCommit => Function,
                    onDebugEnter  => Function,
                    onDebugLeave  => Function
                ) {
                    when(!mod16_native__debug_context_is_allowed()) empty;                    
                    mod16_native__debug_context_bind(
                        a:onDebugPrint, 
                        b:onDebugClear, 
                        d:onDebugCommit, 
                        c:::{
                            when(inDebugContext) empty;
                            print(message:'DEBUG CONTEXT:::ENTERED!!!!!!!!!!!!!!!');
                            inDebugContext = true;
                            onDebugEnter();
                        }, 
                        e:::{
                            print(message:'DEBUG CONTEXT:::EXITED!!!!!!!!!!!!!!!');
                            inDebugContext = false;
                            onDebugLeave();
                        }
                    );
                
                },
                
                enter::() {
                    when(!mod16_native__debug_context_is_allowed()) empty;                    
                    mod16_native__debug_context_enter();
                
                },
                
                query::(expression => String) {
                    when(!mod16_native__debug_context_is_allowed()) empty;                    
                    mod16_native__debug_context_query(a:expression);
                
                }
            
            };
        }
    ).new();
};


@:Project = ::<= {


    return class(
        define:::(this) {
            this.interface = {
                saveSource ::(
                    project => String,
                    name => String,
                    data => String
                ) {
                    when (!package_native__is_packaging_allowed()) empty;
                    package_native__save_source(a:project, b:name, c:data);
                },
                
                makeProject ::(
                    name => String
                ) {                    
                    when (!package_native__is_packaging_allowed()) empty;
                    package_native__make_project(a:name);                    
                },
                
                openSource::(
                    project => String,
                    name => String
                ) {
                    when (!package_native__is_packaging_allowed()) empty;
                    return package_native__open_source(a:project, b: name);
                },
                
                listProjects::(
                    
                ) {
                    when (!package_native__is_packaging_allowed()) empty;
                    return package_native__list_projects();
                
                }
                
                
                
            };
        }
    ).new();
};


@:MOD16 = class(
    name: 'MOD16',
    
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



        
        

        mod16_native__engine_attrib(a:ATTRIBS.UPDATERATE, b:updateRate);
        mod16_native__engine_attrib(a:ATTRIBS.UPDATEFUNC, b:update);
        mod16_native__engine_attrib(a:ATTRIBS.RESOLUTION, b:resolution);
    
        @:allcartIDs = {};
    
    
        this.interface = {     
            // refers to the current ROM
            Cartridge : {
                get :: {
                    // Cartridges should be querried and cached on startup and not 
                    // during calls to the update function. This simplifies the 
                    // environment design greatly.
                    when (!mod16_native__has_boot_context()) error(detail:
                        "MOD16.Cartridge can only be accessed on cartridge boot. Please access and store at the start of your scripts and not during frame updates."
                    );
                    @:cart = mod16_native__get_context_cartridge_id();
                    
                    @out = allcartIDs[cart];
                    when(out != empty) out;
                    
                    out = {
                        Sprite    : Sprite.new(cartID:cart),
                        Palette   : Palette.new(cartID:cart),
                        Tile      : Tile.new(cartID:cart),
                        Background: Background.new(cartID:cart),
                        Audio     : AudioStore.new(cartID:cart),
                        Oscillator: Oscillator.new(cartID:cart),
                        Vertices  : Vertices.new(cartID:cart),
                        
                        subCartridge::(name => String) {
                            return mod16_native__get_sub_cartridge_main(a:cart, b:name);
                        },
                        
                        
                        "import"::(source => String) { // for safety, as the native function directly unsafely retrieves string for speed.
                            return mod16_native__get_source(a:cart, b:source);
                        }
                    };


                    allcartIDs[cart] = out;
                    return out;
                }
            
            },
        
        

            Audio     : {get ::<- Audio},
            Input     : {get ::<- Input},
            RESOLUTION : RESOLUTION,
            Debug     : {get ::<- Debug},
            Project   : {get ::<- Project},
            
            

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
                get ::<- mod16_native__engine_attrib(a:ATTRIBS.CLIPBOARDGET),
                set ::(value => String) <- mod16_native__engine_attrib(a:ATTRIBS.CLIPBOARDSET, b:value)
            },

            
            
            

            
            updateRate : {
                set ::(value => Number) {
                    updateRate = value;
                    mod16_native__engine_attrib(a:ATTRIBS.UPDATERATE, b:updateRate);                    
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





return MOD16;

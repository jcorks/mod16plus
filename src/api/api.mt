@:class = import(module:'Matte.Core.Class');


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
                        out->push(value:ses_native__tile_query(a:index));
                    });
                    ses_native__tile_attrib(a:index, b:ATTRIBS.UNBIND);
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
        KEY_DOWN : 2
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
                addCallback ::(
                    device => Number,
                    callback => Function
                ){
                    if (device < 0 || device > DEVICES.GAMEPAD3) error(detail:'Unrecognized device');
                    return ses_native__input_attrib(a:ACTIONS.ADD, b:device, c:callback); 
                },
                
                
                removeCallback ::(id => Number) {
                    ses_native__input_attrib(a:ACTIONS.REMOVE, b:id);
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
// in rows, where each row is 16 tiles with 8 rows 
// per background.
//
// NOTE: background tiles CANNOT be used as 
// sprites.
@:Background = ::<= {
    @bgIDPool = [];
    @bgID = 0;
    
    @:ATTRIBS = {
        ENABLE:    0,
        POSITIONX :1,
        POSITIONY :2,
        LAYER     :3,
        TILEINDEX :4,
        EFFECT    :5,
        PALETTE   :6
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
            EFFECTS: EFFECTS
        },
        define:::(this) {
            @shown = true;
            @id;
            @positionX = 0;
            @positionY = 0;
            @layer = 0;

            @effect = 0;
            @paletteIndex = 0;
            
            


            this.constructor = ::(index => Number) {
                id = index;
                ses_native__bg_attrib(a:id, b:ATTRIBS.ENABLE,    c:1);
                ses_native__bg_attrib(a:id, b:ATTRIBS.POSITIONX, c:0);
                ses_native__bg_attrib(a:id, b:ATTRIBS.POSITIONY, c:0);
                ses_native__bg_attrib(a:id, b:ATTRIBS.LAYER,     c:0);
                ses_native__bg_attrib(a:id, b:ATTRIBS.EFFECT,    c:0);
                ses_native__bg_attrib(a:id, b:ATTRIBS.PALETTE,   c:0);
                
                return this;
            };

            this.interface = {
                // degree rotation
                show : {
                    set ::(value => Boolean) {
                        shown = value;
                        ses_native__bg_attrib(a:id, b:ATTRIBS.ENABLE, c:if(value == true) 1 else 0);
                    },
                    
                    get ::<- shown
                },            
            

                // scale X relative to center
                x : {
                    set ::(value => Number) {
                        positionX = value;
                        ses_native__bg_attrib(a:id, b:ATTRIBS.POSITIONX, c:value);
                    },
                    
                    get ::<- positionX
                },

                // scale Y relative to center
                y : {
                    set ::(value => Number) {
                        positionX = value;
                        ses_native__bg_attrib(a:id, b:ATTRIBS.POSITIONY, c:value);
                    },
                    
                    get ::<- positionY
                },                


                // the ordering layer. 0-16. Higher means on top
                layer : {
                    set ::(value => Number) {
                        layer = value;
                        ses_native__bg_attrib(a:id, b:ATTRIBS.LAYER, c:value);
                    },
                    
                    get ::<- layer
                },  



                // effect
                effect : {
                    set ::(value => Number) {
                        effect = value;
                        ses_native__bg_attrib(a:id, b:ATTRIBS.EFFECT, c:value);                        
                    },
                    
                    get ::<- effect
                },

                // palette index.
                // palettes are not objects, just handle IDs in SES
                palette : {
                    set ::(value => Number) {
                        paletteIndex = value;
                        ses_native__bg_attrib(a:id, b:ATTRIBS.PALETTE, c:value);                        
                    },
                    
                    get ::<- paletteIndex
                },

                dispose :: {
                    ses_native__bg_attrib(a:id, b:ATTRIBS.ENABLE, c:0);
                    bgIDPool->push(value:id);
                    id = -1;
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
    @:spriteLimit = 4096;
    
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
                    scaleX,
                    scaleY,
                    centerX,
                    centerY,
                    x,
                    y,
                    rotation,
                    layer,
                    effect,
                    palette
                ) {
                    if (show != empty) 
                        ses_native__sprite_attrib(a:index, b:ATTRIBS.ENABLE, c:if((show => Boolean) == true) 1 else 0);

                    if (tile != empty)
                        ses_native__sprite_attrib(a:index, b:ATTRIBS.TILEINDEX, c:tile=>Number);            

                    if (scaleX != empty)
                        ses_native__sprite_attrib(a:index, b:ATTRIBS.SCALEX, c:scaleX=>Number);

                    if (scaleY != empty)
                        ses_native__sprite_attrib(a:index, b:ATTRIBS.SCALEY, c:scaleY=>Number);

                    if (x != empty)
                        ses_native__sprite_attrib(a:index, b:ATTRIBS.POSITIONX, c:x=>Number);

                    if (y != empty)
                        ses_native__sprite_attrib(a:index, b:ATTRIBS.POSITIONY, c:y=>Number);

                    if (rotation != empty)
                        ses_native__sprite_attrib(a:index, b:ATTRIBS.ROTATION, c:rotation=>Number);

                    if (centerX != empty)
                        ses_native__sprite_attrib(a:index, b:ATTRIBS.CENTERX, c:centerX=>Number);

                    if (centerY != empty)
                        ses_native__sprite_attrib(a:index, b:ATTRIBS.CENTERY, c:centerY=>Number);


                    if (layer != empty)
                        ses_native__sprite_attrib(a:index, b:ATTRIBS.LAYER, c:layer=>Number);            

                    if (effect != empty)                        
                        ses_native__sprite_attrib(a:index, b:ATTRIBS.EFFECT, c:effect=>Number);                        

                    if (palette != empty)
                        ses_native__sprite_attrib(a:index, b:ATTRIBS.PALETTE, c:palette=>Number);

                },
                
                EFFECTS: {
                    get::<-EFFECTS
                }
            };
        }
    ).new();
};


return class(
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
            
            // Loads tiles corresponding to ASCII characters
            // using a default font. This can be useful for debugging 
            // or generalized tool development.
            //
            // The tile data is overwritten starting 
            // at the offset given in order of 
            // ascii characters. 
            // For example, with an offset of 0,
            // the tile representing 'A' will be placed
            
            loadAsciiFont::(offset => Number) {
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

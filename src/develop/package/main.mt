@:SES = import(module:'SES.Core');




SES.loadAsciiFont(offset:0);

SES.Palette.set(
    index: 0,
    colors: [
        [1, 0, 0],
        [0, 1, 0],
        [0, 0, 1],
        [1, 1, 1]
    ]
);


@:drawString::(string, x, y) {
    @spr = 0;
    
    @chX = 0;
    @chY = 0;


    @:drawChar::(px, py, code) {
        SES.Sprite.set(
            index: spr,
            tile: code,
            show:true,
            scaleX:1,
            scaleY:1,
            centerX: 0,
            centerY: 0,
            x: px,
            y: py,
            effect: SES.Sprite.EFFECTS.Color
        );
        spr += 1;
    };
    
    [0, string->length]->for(do:::(i) {
        drawChar(
            px: chX * 6 + x,
            py: chY * 8 + y,
            code: string->charCodeAt(index:i)
        );
        
        if (string->charAt(index:i) == '\n' || chX > 30) ::<= {
            chY += 1;
            chX = 0;
        } else ::<= {
            chX += 1;
        };
        
    });
};



drawString(x:0, y:0, string:
"
static GLint ses_sdl_gl_get_tile_attribs(uint32_t id, float * u, float * v, float * unit) {
    *u = 0;
    *v = 0;
    if (id > TILE_SPRITE_MAX_ID)
        return 1;
    int slot = id / TILES_PER_TEXTURE;
    int index = id % TILES_PER_TEXTURE;        
    SES_GLTileTexture * t = gl.textures[slot];
    if (t == NULL)
        return 1;
    
    float TILES_PER_ROW = (REAL_TEX_SIZE / TILE_SIZE);
        
    *u = (index % TILES_PER_TEXTURE) / TILES_PER_ROW;
    *v = (index / TILES_PER_TEXTURE) / TILES_PER_ROW;
    *unit = 1 / TILES_PER_ROW;
    return t->texture;
}
"
    
);





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


/*

SES.Sprite.set(
    index: 0,

    tile: 'A'->charCodeAt(index:0), 
    show: true,
    scaleX: 5,
    scaleY: 5,
    centerX: -4,
    centerY: -4,
    
    x: 20,
    y: 20,
    
    effect: SES.Sprite.EFFECTS.COLOR
);  



SES.Sprite.set(
    index: 1,

    tile: 'B'->charCodeAt(index:0) ,       
    show: true,
    scaleX: 5,
    scaleY: 5,
    centerX: -4,
    centerY: -4,
    
    x: 40,
    y: 40,
    
    effect: SES.Sprite.EFFECTS.COLOR
);  









SES.Tile.copy(from:0, to:0x40000);
SES.Tile.copy(from:1, to:0x40001);
SES.Tile.copy(from:0, to:0x40002);
SES.Tile.copy(from:1, to:0x40003);
SES.Tile.copy(from:0, to:0x40004);
SES.Tile.copy(from:1, to:0x40005);

@:bg = SES.Background.new(index:0);
bg.palette = 0;
@counter = 0;
SES.update = ::{
    print(message:'Test!' + counter);
    counter += 1;
    
    
    
    SES.Sprite.set(index:0, rotation:counter*3);
    SES.Sprite.set(index:1, rotation:counter*5);
};

*/

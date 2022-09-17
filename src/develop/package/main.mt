@:SES = import(module:'SES.Core');


SES.Tile.set(
    index: 0,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 3, 3, 0, 0, 0,
        0, 0, 0, 3, 3, 0, 0, 0,
        0, 0, 3, 2, 1, 3, 0, 0,
        0, 0, 3, 1, 2, 3, 0, 0,
        0, 3, 1, 1, 1, 2, 3, 0,
        0, 3, 2, 2, 2, 2, 3, 0,
        3, 3, 3, 3, 3, 3, 3, 3
    ]
);



SES.Tile.set(
    index: 1,
    data: [
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 2, 2, 2, 2, 2, 2, 1,
        1, 2, 3, 3, 3, 3, 2, 1,
        1, 2, 3, 4, 4, 3, 2, 1,
        1, 2, 3, 4, 4, 3, 2, 1,
        1, 2, 3, 3, 3, 3, 2, 1,
        1, 2, 2, 2, 2, 2, 2, 1,
        1, 1, 1, 1, 1, 1, 1, 1
    ]
);


SES.Palette.set(
    index: 0,
    colors: [
        [1, 0, 0],
        [0, 1, 0],
        [0, 0, 1],
        [1, 0, 1]
    ]
);


@:makeSprite = ::{
    @:s = SES.Sprite.new();
    s.tile = 0;
    s.scaleX = 5;
    s.scaleY = 5;

    s.centerX = -4;
    s.centerY = -4;
    
    s.effect = SES.Sprite.EFFECTS.BLEND;
    
    return s;
};

@counter = 0;

@a = makeSprite();
a.x = 20; a.y = 20;

@s = makeSprite();
s.x = 20; s.y = 40;
s.tile = 1;








SES.Tile.copy(from:0, to:0x40000);
SES.Tile.copy(from:1, to:0x40001);
SES.Tile.copy(from:0, to:0x40002);
SES.Tile.copy(from:1, to:0x40003);
SES.Tile.copy(from:0, to:0x40004);
SES.Tile.copy(from:1, to:0x40005);

@:bg = SES.Background.new(index:0);
bg.palette = 0;
SES.update = ::{
    print(message:'Test!' + counter);
    counter += 3;
    
    s.rotation += 3;
    a.rotation += 5;
};

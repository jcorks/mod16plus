@:SES = import(module:'SES.Core');


SES.Tile.set(
    index: 0,
    data: [
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 1, 1, 1, 0, 0,
        0, 0, 1, 1, 1, 0, 0, 0,
        0, 0, 1, 1, 1, 0, 0, 0,
        0, 0, 1, 1, 1, 0, 0, 0,
        0, 1, 1, 1, 1, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
    ]
);


SES.Palette.set(
    index: 0,
    colors: [
        '#ff0000',
        '#00ff00',
        '#0000ff',
        '#ff00ff'
    ]
);

@:s = SES.Sprite.new();
s.tile = 0;
s.scaleX = 8;
s.scaleY = 8;

s.centerX = -4;
s.centerY = -4;
//s.y = 20;
//s.x = 20;


@counter = 0;
SES.update = ::{
    print(message:'Test!' + counter);
    counter += 1;
    
    s.rotation += 1;
};

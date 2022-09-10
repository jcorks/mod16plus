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

@:s = SES.Sprite.new();
s.tile = 0;

@counter = 0;
SES.update = ::{
    print(message:'Test!' + counter);
    counter += 1;
    
    if (counter % 50 == 0)
        SES.updateRate *= 2;
        
    s.rotation += 1;
};

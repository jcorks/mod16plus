@:SES = import(module:'SES.Core');
@:Fetcher = import(module:'fetcher.mt');


@TILE_BG = Fetcher.Tile.newID();
@TILE_FG = Fetcher.Tile.newID();
@PALETTE_GRAY = Fetcher.Palette.newID();



@BACKGROUND_TOP0 = Fetcher.Background.newID();
@BACKGROUND_TOP1 = Fetcher.Background.newID();


@BACKGROUND_BOTTOM0 = Fetcher.Background.newID();
@BACKGROUND_BOTTOM1 = Fetcher.Background.newID();


SES.Tile.set(
    index: TILE_BG,
    data : [
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1
    ]
);



SES.Tile.set(
    index: TILE_FG,
    data : [
        4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4
    ]
);

SES.Palette.set(
    index: PALETTE_GRAY,
    colors: [
        [0, 0, 0],
        [0.3, 0.3, 0.3],
        [0.6, 0.6, 0.6],
        [1, 1, 1]
    ]
);





@:setTopBar::{

    // sets tile background
    [0, 16]->for(do:::(i) {
        SES.Tile.copy(from:TILE_BG, to: Fetcher.backgroundIDtoTileID(id:BACKGROUND_TOP0)+i);
    });
    SES.Background.set(index:BACKGROUND_TOP0, show:true, x:0, y:0, layer:0, effect:SES.Background.EFFECTS.COLOR, palette:PALETTE_GRAY);

    // sets tile background
    [0, 16]->for(do:::(i) {
        SES.Tile.copy(from:TILE_BG, to: Fetcher.backgroundIDtoTileID(id:BACKGROUND_TOP1)+i);
    });
    SES.Background.set(index:BACKGROUND_TOP1, show:true, x:16*8, y:0, layer:0, effect:SES.Background.EFFECTS.COLOR, palette:PALETTE_GRAY);






    // sets tile background
    [0, 16]->for(do:::(i) {
        SES.Tile.copy(from:TILE_BG, to: Fetcher.backgroundIDtoTileID(id:BACKGROUND_BOTTOM0)+i);
    });
    SES.Background.set(index:BACKGROUND_BOTTOM0, show:true, x:0, y:19*8, layer:0, effect:SES.Background.EFFECTS.COLOR, palette:PALETTE_GRAY);

    // sets tile background
    [0, 16]->for(do:::(i) {
        SES.Tile.copy(from:TILE_BG, to: Fetcher.backgroundIDtoTileID(id:BACKGROUND_BOTTOM1)+i);
    });
    SES.Background.set(index:BACKGROUND_BOTTOM1, show:true, x:16*8, y:19*8, layer:0, effect:SES.Background.EFFECTS.COLOR, palette:PALETTE_GRAY);

};



setTopBar();

/*
    Singleton-based ID transaction system
    Makes it easy to instance your IDs!
*/

@:class = import(module:'Matte.Core.Class');

@:Fetcher = class(
    name: 'Fetcher',
    define:::(this) {
        @count = 0;


        this.interface = {
        
            // Fetches the next free ID
            newID :: {
                count += 1;
                return count;
            },
            
            // Skips the next count IDs. Used 
            // to simulate claiming groups of IDs 
            // that are off-limits to other users.
            claimIDs::(amount) {
                count += amount;
            }
        };
    }
);


return {
    // Fetcher for sprite IDs
    Sprite : Fetcher.new(),
    
    // Fetcher for background IDs
    Background : Fetcher.new(),
    
    // Fetcher for IDs.
    Palette : Fetcher.new(),
    
    // Fetcher for (sprite) IDs. Only "makes sense" 
    // up to 0x40000.
    Tile : Fetcher.new(),
    
    
    // Convenience function that gives the local tile ID 
    // that the given background starts at.
    // Backgrounds are always 16 x 8 tiles.
    backgroundIDtoTileID::(id) {
        return 0x40000 + (id * 32*16);
    }
};

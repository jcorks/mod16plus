@:SES = import(module:'SES.Core');



SES.Palette.set(
    index: 0,
    colors: [
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [1, 0.9, 0.8]

    ]
);


SES.Palette.set(
    index: 1,
    colors: [
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [1, 0, 0.5]

    ]
);


SES.Palette.set(
    index: 2,
    colors: [
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0.5, 1]

    ]
);


SES.Palette.set(
    index: 3,
    colors: [
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 1, 0.5]

    ]
);

SES.Palette.set(
    index: 4,
    colors: [
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0.5, 0.5, 0.5]

    ]
);

SES.Palette.set(
    index: 5,
    colors: [
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [1, 0.70, 0.5]

    ]
);

SES.Palette.set(
    index: 6,
    colors: [
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0.6, 0.6, 0.6]

    ]
);

SES.Palette.set(
    index: 7,
    colors: [
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [1, 1, 0.5]

    ]
);

@:searches = [
    {token: 'if', palette: 3},
    {token: 'else', palette: 3},
    {token: 'return', palette: 3},
    {token: 'when', palette: 3},
    {token: 'match', palette: 3},
    {token: 'default', palette: 3},
    {token: 'foreach', palette: 3},
    {token: 'for', palette: 3},
    {token: '=>', palette: 3},
    {token: '<-', palette:3},


    {token: ':::', palette:2},
    {token: '::', palette:2},
    {token: ':', palette:2},
    {token: 'Function', palette:2},
    {token: 'Object', palette:2},
    {token: 'Number', palette:2},
    {token: 'Empty', palette:2},
    {token: 'String', palette:2},
    {token: '::<=', palette:2},
    
    {token: '@', palette:1},
    {token: 'empty', palette: 1},

    {token: '->', palette:5},
    {token: 'length', palette:5},
    {token: 'keycount', palette:5},
    {token: 'find', palette:5},
    {token: 'search', palette:5},
    {token: 'parse', palette:5},
    {token: 'random', palette:5},
    {token: 'floor', palette:5},
    {token: 'ceil', palette:5},
    {token: 'round', palette:5},

    {token: '.', palette:6},
    {token: '}', palette:6},
    {token: '{', palette:6},
    {token: '~', palette:6},
    {token: '%', palette:6},
    {token: '^', palette:6},
    {token: '&', palette:6},
    {token: '*', palette:6},
    {token: '(', palette:6},
    {token: ')', palette:6},
    {token: '-', palette:6},
    {token: '+', palette:6},
    {token: '=', palette:6},
    {token: '[', palette:6},
    {token: ']', palette:6},
    {token: "'", palette:6},
    {token: '"', palette:6},
    {token: '/', palette:6},
    {token: '?', palette:6},
    {token: '<', palette:6},
    {token: '>', palette:6},
    {token: ',', palette:6},
    {token: '|', palette:6}



];


@:applySearches::(string) {
    searches->foreach(do:::(i, search) {
        @:ind = string->search(key:search.token);
        when(ind == -1) empty;
        
        textarea.setColor(
            paletteID: search.palette,
            fromY: textarea.cursorY,
            toY: textarea.cursorY,
            fromX: ind,
            toX: ind+search.token->length-1
        );
        
    
    });
};


@textarea = SES.Text.createArea(onChange:: {

    

    @:line = textarea.getLine(index:textarea.cursorY);
    textarea.setColor(
        paletteID: 0,
        fromY: textarea.cursorY,
        toY: textarea.cursorY,
        fromX: 0,
        toX: line->length-1
    );




    @lastWhitespace = -1;
    [::] {
        [0, line->length]->for(do:::(i) {
        
            
        
        
            match(line->charAt(index:i)) {
              (' ', '\n', '\t', '\r'): ::<= {
                @:token = line->substr(from:lastWhitespace+1, to:i-1);
                lastWhitespace = i;
              },
              
              ('('):::<= {
                if (i > 0 && match(line->charAt(index:i-1)) {
                    ('-', '+', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', ':', ';', '"', "'", '/', '?', '<', '>', ',', '.', '=', '[', ']', '{', '}', '|'): false,
                    default: true
                })
                    textarea.setColor(paletteID:7, 
                        fromX: lastWhitespace+1,
                        toX: i-1,
                        fromY: textarea.cursorY,
                        toY: textarea.cursorY
                    );
                
              } 
            };
            
            
            if (i < line->length-1 &&
                line->substr(from:i, to:i+1) == '//'
            ) ::<= {
                textarea.setColor(paletteID:4, 
                    fromX: i,
                    toX: line->length-1,
                    fromY: textarea.cursorY,
                    toY: textarea.cursorY
                );
                send();
                  
            };

        });

        applySearches(string:line);  

    };

});


// full GBA screen
textarea. = {
    widthChars  : 40,
    heightChars : 20,    
    editable    : true,
    scrollable  : true
};







SES.Input.addCallback(
    device:SES.Input.DEVICES.POINTER0,
    callback:::(event, x, y, button) {
        if (event == SES.Input.EVENTS.POINTER_BUTTON_DOWN) 
            SES.debug();
    }
);




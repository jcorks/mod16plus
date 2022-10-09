@:SES = import(module:'SES.Core');





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
    [0, line->length]->for(do:::(i) {
    
        
    
    
        match(line->charAt(index:i)) {
          (' ', '\n', '\t', '\r'): ::<= {
            @:token = line->substr(from:lastWhitespace+1, to:i-1);
            match(token) {
              ('if', 'else', 'return', '=>', 'when', 'match', 'default'): ::<={
                textarea.setColor(paletteID:3, 
                    fromX: lastWhitespace+1,
                    toX: i-1,
                    fromY: textarea.cursorY,
                    toY: textarea.cursorY
                );
              }
            };
        
          
          
            lastWhitespace = i;
          }
        };
    
        if (line->charAt(index:i) == '@')
            textarea.setColor(
                paletteID: 1,
                fromY: textarea.cursorY,
                toY: textarea.cursorY,
                fromX: i,
                toX: i
            );
            
        if (line->charAt(index:i) == ':')
            textarea.setColor(
                paletteID: 2,
                fromY: textarea.cursorY,
                toY: textarea.cursorY,
                fromX: i,
                toX: i
            );    
    });



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




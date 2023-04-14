    $(".dropdown-toggle").next(".dropdown-menu").children().on("click",function(){        
        $(this).closest(".dropdown-menu").prev(".dropdown-toggle").text($(this).text());
    });

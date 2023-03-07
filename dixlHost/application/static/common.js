$("#nav-lock").click(function(event) {
                        // Current lock state
                        locker = $("#nav-lock").children(".bi")
                        state =locker.hasClass("bi-unlock-fill");
                        if (state) {
                            $("#layout-selector").attr("disabled","disabled");
                            locker.removeClass("bi-unlock-fill").addClass("bi-lock-fill");
                        } else {
                            $("#layout-selector").removeAttr("disabled");
                            locker.removeClass("bi-lock-fill").addClass("bi-unlock-fill");
                        }
                        // Cycle others tools
                        $(".nav-tools .btn").each(  function () {
                                                        // Skip lock
                                                        if ($(this).data("function") != "lock") {
                                                            if (state)
                                                                $(this).addClass("disabled");
                                                            else
                                                                $(this).removeClass("disabled");
                                                        }

                                                    }
                        );

                        return event.preventDefault();
                    });

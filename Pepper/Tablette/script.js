


session = new QiSession();

session.service("ALMemory").done(function(ALMemory) {

    ALMemory.subscriber("SimpleWeb/Page/Start").done(function(subscriber) {
        subscriber.signal.connect(function() {
            $('#section_start').addClass('active').siblings('.section').removeClass('active');
        });
    });

    ALMemory.subscriber("SimpleWeb/Page/Progress0").done(function(subscriber) {
        subscriber.signal.connect(function() {
            $('#section_0').addClass('active').siblings('.section').removeClass('active');
        });
    });

    

    ALMemory.subscriber("SimpleWeb/Page/Progress33").done(function(subscriber) {
        subscriber.signal.connect(function() {
            $('#section_33').addClass('active').siblings('.section').removeClass('active');
        });
    });



    ALMemory.subscriber("GameStart").done(function(subscriber) {
        subscriber.signal.connect(function(timeValue) {
            console.log("GameStart reçu avec la valeur : " + timeValue); // Vérifie la valeur
    
            // Récupère tous les éléments ayant la classe "clock"
            var clockElements = document.getElementsByClassName("clock");
    
            // Vérifie s'il existe des éléments avec cette classe
            if (clockElements.length > 0) {
                // Parcourt chaque élément pour mettre à jour son contenu
                for (var i = 0; i < clockElements.length; i++) {
                    clockElements[i].innerHTML = timeValue;
                }
            } else {
                console.error("Aucun élément avec la classe 'clock' trouvé.");
            }
        if (timeValue == "00:01") {
            session.service("ALMemory").done(function(ALMemory) {
                ALMemory.raiseEvent("timeout", 1);
                console.log("Événement Perdu émis");
            }).fail(function(error) {
                console.error("Erreur lors de l'émission de l'événement Perdu :", error);
            });
        }
    });
});

    ALMemory.subscriber("SimpleWeb/Page/Progress66").done(function(subscriber) {
        subscriber.signal.connect(function() {
            $('#section_66').addClass('active').siblings('.section').removeClass('active');
        });
    });

    ALMemory.subscriber("SimpleWeb/Page/Progress99").done(function(subscriber) {
        subscriber.signal.connect(function() {
            $('#section_99').addClass('active').siblings('.section').removeClass('active');
        });
    });

    ALMemory.subscriber("SimpleWeb/Page/Code").done(function(subscriber) {
        subscriber.signal.connect(function() {
            $('#section_code').addClass('active').siblings('.section').removeClass('active');
        });
    });

    ALMemory.subscriber("SimpleWeb/Page/Reussi").done(function(subscriber) {
        subscriber.signal.connect(function() {
            $('#section_final').addClass('active').siblings('.section').removeClass('active');
        });
    });
    
    ALMemory.subscriber("SimpleWeb/Page/Perdu").done(function(subscriber) {
        subscriber.signal.connect(function() {
            $('#section_perdu').addClass('active').siblings('.section').removeClass('active');
        });
    });

    ALMemory.subscriber("SimpleWeb/Page/FinalButton").done(function(subscriber) {
        subscriber.signal.connect(function() {
            $('#section_bouton').addClass('active').siblings('.section').removeClass('active');
        });
    });

    // Abonnement à l'événement "Validatecode" pour gérer l'affichage du bouton
    ALMemory.subscriber("ValidatecodeCapteur").done(function(subscriber) {
        subscriber.signal.connect(function() {
            // Lorsque l'événement est levé, afficher le bouton
            $('#code_button_capteur').show();
        });
    });

    // Abonnement à l'événement "Validatecode" pour gérer l'affichage du bouton
    ALMemory.subscriber("ValidatecodeDetection").done(function(subscriber) {
        subscriber.signal.connect(function() {
            console
            // Lorsque l'événement est levé, afficher le bouton
            $('#code_button_detection').show();
        });
    });
    // Abonnement à l'événement "Validatecode" pour gérer l'affichage du bouton
    ALMemory.subscriber("ValidatecodeIA").done(function(subscriber) {
        subscriber.signal.connect(function() {
            // Lorsque l'événement est levé, afficher le bouton
            $('#code_button_ia').show();
        });
    });

    ALMemory.subscriber("SimpleWeb/Page/CodeCorrectCapteur").done(function(subscriber) {
        subscriber.signal.connect(function() {
            console.log("Code correct pour la section Capteur");
        });
    } 
    );

    ALMemory.subscriber("Reset").done(function(subscriber) {
        subscriber.signal.connect(function() {
            // Lorsque l'événement est levé, afficher le bouton
            $('#code_button_capteur').hide();
            $('#code_button_detection').hide();
            $('#code_button_ia').hide();
            capteurValidated = false;
            detectionValidated = false;
            iaValidated = false;
        });
    });

    // Masquer le bouton au départ
    $('#code_button').hide();
            
    function subscribeToSection(sectionId) {
        ALMemory.subscriber("SimpleWeb/Page/" + sectionId).done(function(subscriber) {
            subscriber.signal.connect(function() {
                $('#' + sectionId).addClass('active').siblings('.section').removeClass('active');
            });
        });
    }

    subscribeToSection('Progress0'); 
    subscribeToSection('Progress33'); 
    subscribeToSection('Progress66'); 
    subscribeToSection('Progress99'); 
    subscribeToSection('Code');      

    $('.custom-button').click(function() {
        $('#section_code').addClass('active').siblings('.section').removeClass('active');
        console.log("click");
    });
    
});

// Fonction pour émettre l'événement en fonction du code validé
function emitEvent(codeType) {
    switch (codeType) {
        case "Capteur":
            session.service("ALMemory").done(function(ALMemory) {
                console.log("Service ALMemory est prêt, émission de l'événement...");
                ALMemory.raiseEvent("SimpleWeb/Page/CodeCorrectCapteur", 1);
                console.log("Événement SimpleWeb/Page/CodeCorrectCapteur émis");
            }).fail(function(error) {
                console.error("Erreur lors de l'initialisation du service ALMemory :", error);
            });
            break;

        case "Detection":
            session.service("ALMemory").done(function(ALMemory) {
                console.log("Service ALMemory est prêt, émission de l'événement...");
                ALMemory.raiseEvent("SimpleWeb/Page/CodeCorrectDetection", 1);
                console.log("Événement SimpleWeb/Page/CodeCorrectDetection émis");
            }).fail(function(error) {
                console.error("Erreur lors de l'initialisation du service ALMemory :", error);
            });
            break;
        case "IA":
            session.service("ALMemory").done(function(ALMemory) {
                console.log("Service ALMemory est prêt, émission de l'événement...");
                ALMemory.raiseEvent("SimpleWeb/Page/CodeCorrectIA", 1);
                console.log("Événement SimpleWeb/Page/CodeCorrectIA émis");
            }).fail(function(error) {
                console.error("Erreur lors de l'initialisation du service ALMemory :", error);
            });
            break;
        case "Final":
            session.service("ALMemory").done(function(ALMemory) {
                console.log("Service ALMemory est prêt, émission de l'événement...");
                ALMemory.raiseEvent("SimpleWeb/Page/CodeCorrectFinal", 1);
                console.log("Événement SimpleWeb/Page/CodeCorrectFinal émis");
            }).fail(function(error) {
                console.error("Erreur lors de l'initialisation du service ALMemory :", error);
            });
            break            
        default:
            console.error("Type de code inconnu :", codeType);
    }}

    

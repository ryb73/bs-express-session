open Express;

ExpressSession.make @@ ExpressSession.opts
    secret::"secret" name::"sess" proxy::false resave::true
    rolling::false saveUninitialized::true
    genid::(fun req => Request.hostname req)
    cookie::(ExpressSession.cookieOpts
        domain::"a.com" httpOnly::false maxAge::10
        path::"/web" secure::true ()
    )
();

type session = {
    id: int,
    name: string
};
module Session = ExpressSession.Make({
    type t = session;
    let key = "data";
});

let app = App.make ();

App.get app path::"/" @@ Middleware.from @@ fun req resp _ => {
    Session.set req {
        id: 24,
        name: "me"
    };

    let d = Session.get req;
    switch d {
        | None => Js.log "none"
        | Some { id, name } => {
            Js.log id;
            Js.log name;
        }
    };

    Response.sendString resp "ok";
};
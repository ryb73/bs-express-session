open Express;
open Option;
open Js.Result;

let flip = BatPervasives.flip;

let _resultToOpt = (r) =>
    switch r {
        | Ok(v) => Some(v)
        | _ => None
    };

type cookieOpts;

[@bs.obj]
external _cookieOpts : (
    ~domain: string=?, ~httpOnly: Js.boolean=?, ~maxAge: int=?,
    ~path: string=?, ~secure: Js.boolean=?, unit
) => cookieOpts = "";

let _maybeJsBool = (opt) =>
    switch opt {
        | Some(b) => Some(Js.Boolean.to_js_boolean(b))
        | None => None
    };

let cookieOpts = (~domain=?, ~httpOnly=?, ~maxAge=?, ~path=?, ~secure=?) =>
    _cookieOpts(~domain?, ~maxAge?, ~path?, ~httpOnly=?_maybeJsBool(httpOnly),
        ~secure=?_maybeJsBool(secure));

type opts;

[@bs.obj]
external _opts : (
    ~secret: string,
    ~cookie: cookieOpts=?,
    ~genid: Request.t => string=?,
    ~name: string=?,
    ~proxy: Js.boolean=?,
    ~resave: Js.boolean=?,
    ~rolling: Js.boolean=?,
    ~saveUninitialized: Js.boolean=?,
    unit
) => opts = "";

let opts = (
    ~secret, ~cookie=?, ~genid=?, ~name=?, ~proxy=?,
    ~resave=?, ~rolling=?, ~saveUninitialized=?
) =>
    _opts(~secret, ~cookie?, ~genid?, ~name?,
        ~proxy=?_maybeJsBool(proxy), ~resave=?_maybeJsBool(resave),
        ~rolling=?_maybeJsBool(rolling),
        ~saveUninitialized=?_maybeJsBool(saveUninitialized)
    );

[@bs.module] external make : opts => Middleware.t = "express-session";

module type Config = {
    [@autoserialize] type t;
    let key: string;
};

module Make = (C: Config) => {
    let _decodeSession = (req) => {
        let reqData = Request.asJsonObject(req);
        let optSession = Js.Dict.get(reqData, "session");
        switch optSession {
            | None => None
            | Some(jsonSession) => Js.Json.decodeObject(jsonSession)
        }
    };

    let set = (req, value) =>
        switch (_decodeSession(req)) {
            | None => false
            | Some(session) =>
                Js.Dict.set(session, C.key, C.t__to_json(value));
                true
        };

    let get = (req) =>
        switch (_decodeSession(req)) {
            | None => None
            | Some(session) =>
              Js.Dict.get(session, C.key)
                  |> flip(bind, (json) => _resultToOpt(C.t__from_json(json)))
        };
};

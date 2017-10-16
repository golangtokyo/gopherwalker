= Go APIのモバイル認証処理

== はじめに

初めまして、株式会社Gunosyの新規事業開発室でエンジニアをしております、timakinです。Gunosyでは主にGoのAPIとSwiftのiOSクライアントサイド開発を担当しております。

弊社のプロダクトもそうですが、モバイルアプリでは起動直後に一見特段の認証処理をせずにコンテンツを表示します。

しかし、裏側ではデバイス認証をしておき、正確にユーザー情報を管理しなければならない場合というのがあります。

この章では、上記のようなGoをモバイルファーストのプロダクトで用いる際の実装方法について書いていこうと思います。

なお、今回対象とするのはOAuth認証などではなく、より単純なデバイスの認証とします。

また、環境はGoogle App Engineで用意し、Goのバージョンは1.8とします。

== 認証処理で必要なこと

認証処理というのは往々にして実装が複雑になりがちです。初回リクエスト時に認証処理として具体的にやるべきことは、たとえば以下のようなことです。

 * iOS, Android両方を考慮した値の受け付け
 * 既存ユーザーかどうかをチェック
 * BANされていないかをチェック
 * バージョンを示す文字列やOS種別をレコード時に形式変換
 * 有効なABテスト情報を格納
 * コンテンツ初期表示に必要な情報を格納
 * 取得したユーザー情報をJWTとしてシリアライズ

これらを全て一つのリクエストのうちに済ませようとすると、REST APIを提供していても、このリクエストのみJSON-RPCといっても差し支えないほどに込み入った処理をすることになります。

さらには、そういった処理の中ではトランザクションを適切に張る必要もあり、他のリソース返却のAPIよりも難易度が上がります。

プロダクトが成長するにつれ、チュートリアル処理やキャンペーン情報など、様々な情報をこの初期化処理のレスポンスペイロードに入れ始め、混沌としたAPIが出来上がると思います。

今回はそこまでいく前の、シンプルなデバイス認証をどうやってGoで実装すべきかについて考えてみます。

== main.go

起動スクリプトは以下のような書き方になると思います。

//list[main.go][起動スクリプトの例]{
package main

import (
	_ "google.golang.org/appengine/remote_api"

	"net/http"

	"github.com/fukata/golang-stats-api-handler"
	"github.com/gorilla/mux"
	"github.com/justinas/alice"
	"github.com/timakin/sample_api/services/api/src/domain/auth"
	"github.com/timakin/sample_api/services/api/src/middleware"
	"github.com/timakin/sample_api/services/api/src/repository"
)

func init() {
	h := initHandlers()
	http.Handle("/", h)
}

func initHandlers() http.Handler {
	// Routing
	r := mux.NewRouter()

	// middleware chain
	chain := alice.New(
		middleware.AccessControl,
		middleware.Authenticator,
	)

	// cpu, memory, gc, etc stats
	r.HandleFunc("/api/stats", stats_api.Handler)

	// Authentication Service
	authRepository := repository.NewAuthRepository()
	authService := auth.NewService(authRepository)
	authDependency := &auth.Dependency{
		AuthService: authService,
	}

	r = auth.MakeInitHandler(authDependency, r)

	// Bind middlewares
	h := chain.Then(r)

	return h
}
//}

appengine特有のpackageが一部読み込まれていますが、基本的な書き方は変わりません。

特筆すべき点としては、

 * MakeInitHandlerという関数でルーティング定義
 * Repository, Serviceなどを依存オブジェクトとしてカスタムハンドラに注入している
 * 認証処理をauthというpackageにまとめている

という点でしょうか。1つ目はルーティングの定義を外部の関数に切り出しておくというだけなのですが、見栄えが良いので個人的にオススメです。

2点目は、Goのリクエストハンドラの実装方法です。この点は次の節で詳しく解説いたします。

== ハンドラ

この節ではハンドラについて述べていきます。まずはカスタムハンドラの定義です。

//list[handler.go][カスタムハンドラの定義]{
package auth

import (
	"context"
	"net/http"

	"google.golang.org/appengine"

	"github.com/gorilla/mux"
	"github.com/timakin/sample_api/services/api/src/handler"
)

type Dependency struct {
	AuthService Service
}

type CustomHandler struct {
	Impl func(http.ResponseWriter, *http.Request)
}

func (h CustomHandler) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	vars := mux.Vars(r)
	ctx := appengine.WithContext(r.Context(), r)
	ctx, cancel := context.WithTimeout(ctx, handler.TimeOutLimit)
	defer cancel()
	ctx = handler.SetReqParams(ctx, vars)
	cr := r.WithContext(ctx)
	h.Impl(w, cr)
}
//}

=== なぜカスタムハンドラが必要か

そもそもなぜカスタムハンドラの定義をする必要があるのでしょうか？これはGo特有の話になってきます。

Goの起動スクリプトで初期化されたミドルウェア(LoggerやDBクライアントなど)は、できることなら各リクエストで使いまわしたいところです。

特に気にせず書いてしまうと、packageのグローバル変数として定義したオブジェクトを使ったり、リクエストごとにLoggerを初期化、などとなります。

それはあまりよろしくないので、GoのInterfaceを使って、それらのミドルウェアのオブジェクトをDependency(依存オブジェクト)として持ったハンドラを作成すると便利です。

CustomHandlerがその実装例になります。このstructはServeHTTPメソッドを実装しています。これを実装すればGoはHTTPリクエストハンドラとしてみなしてくれます。

今回の認証処理で使うのは、認証情報へのデータアクセスクライアントなどを持ったRepositoryオブジェクト、をさらに内部に持ったビジネスロジックを実装したService構造体、を持ったDependencyという、ちょっと入れ子構造として多層なオブジェクトを持ったハンドラです。

=== contextについて

さらに、Goのcontextという概念がよく実装で問題になります。これはGoのchannel間でキャンセル処理を統一管理する用途、そしてリクエストに限定的な値(ユーザー情報など)をストアするための用途で使われる機構です。

主にハンドラでどう関係するかといえば、タイムアウト処理で関係してきます。

リクエストがタイムアウトした時に、張ってるトランザクションをキャンセルしつつエラーレスポンスを返さなければなりません。

まさにこのような用途でcontextを使います。例えばDBクライアントに実行メソッドとしてExecというのが用意されていれば、標準パッケージならほぼ確実にExecContextというような、第一引数にcontextを渡せるメソッドが生えています。

これがあれば、タイムアウトと同時にトランザクション処理をキャンセルしたりできます。

なのでタイムアウト設定としてcontextをどこかで設定すべきなのですが、今回はServeHTTPの中で設定することとします。

=== ルーティング

今回はPOSTリクエストとの/initというエンドポイントを、初期の認証処理の入り口としておいて見ます。

//list[routing.go][ルーティング]{
package auth

import "github.com/gorilla/mux"

// MakeInitHandler ... register handlers for initialization
func MakeInitHandler(d *Dependency, r *mux.Router) *mux.Router {
	initHandler := CustomHandler{Impl: d.InitHandler}
	r.Handle("/api/init", initHandler).Methods("POST")
	return r
}
//}

冒頭でルーティング定義を別メソッドに切り出して置くと見栄えが良い的な話をしましたが、ここがその実装です。

カスタムハンドラでラップしつつ、実際のリクエストハンドラに引き渡します。その実際のリクエストハンドラが次の通りです。

//list[handler.go][実際にリクエストを処理するハンドラ]{
package auth

import (
	"net/http"

	"github.com/pkg/errors"
	"github.com/timakin/sample_api/services/api/src/handler"
)

// InitHandler ... user registration and returns userInfo payload
// Path: /init
func (d *Dependency) InitHandler(w http.ResponseWriter, r *http.Request) {
	payload, err := decodeInitRequest(r)
	if err != nil {
		err = errors.Wrap(err, "Failed to parse auth params from path components.")
		res := handler.NewErrorResponse(http.StatusBadRequest, err.Error())
		handler.Redererer.JSON(w, res.Status, res)
		return
	}
	record, isNew, token, err := d.AuthService.Init(r.Context(), &User{
		Device: &Device{
			DUID:       payload.DUID,
			DeviceName: payload.Device,
			OSName:     payload.OS,
			OSVersion:  payload.OSVersion,
			DeviceApp: &DeviceApp{
				AppVersion: payload.AppVersion,
				BundleID:   payload.BundleID,
			},
		},
	})
	if err != nil {
		res := handler.NewErrorResponse(http.StatusInternalServerError, err.Error())
		handler.Redererer.JSON(w, res.Status, res)
		return
	}

	resPayload := encodeInitResponse(record.ID, isNew, token)

	handler.Redererer.JSON(w, http.StatusOK, resPayload)
}

type InitRequestPayload struct {
	DUID       string `json:"duid" validate:"required"`
	AppVersion string `json:"app_version" validate:"required"`
	BundleID   string `json:"bundle_id" validate:"required"`
	Device     string `json:"device" validate:"required"`
	OS         string `json:"os" validate:"required"`
	OSVersion  string `json:"os_version" validate:"required"`
}

func decodeInitRequest(r *http.Request) (*InitRequestPayload, error) {
	payload := InitRequestPayload{
		DUID:       r.FormValue("duid"),
		AppVersion: r.FormValue("app_version"),
		BundleID:   r.FormValue("bundle_id"),
		Device:     r.FormValue("device"),
		OS:         r.FormValue("os"),
		OSVersion:  r.FormValue("os_version"),
	}
	err := handler.Validator.Struct(payload)
	if err != nil {
		return nil, err
	}

	return &payload, nil
}

type InitResponsePayload struct {
	UserID      int64   `json:"user_id"`
	IsNewUser   bool    `json:"is_new_user"`
	AccessToken string  `json:"access_token"`
}

func encodeInitResponse(userID int64, isNewUser bool, at string) *InitResponsePayload {
	payload := InitResponsePayload{
		UserID:      userID,
		IsNewUser:   isNewUser,
		AccessToken: at,
	}
	return &payload
}
//}

受け取ったInit用のリクエストbodyをパースして、ビジネスロジックへの流し込みます。そして結果をレスポンスpayloadに詰めて返却する、というのがおおまかな流れです。

リクエストbodyとしてモバイルクライアント側が送信する値としては、

 * DUID - デバイスのユニークID
 * AppVersion - クライアントにインストールされているアプリバージョン
 * BundleID - 上記と一緒にクライアントに定義されるであろうBundleID
 * Device - デバイス名
 * OS - OS種別
 * OSVersion - OSのバージョン 

などです。広告の配信機構などを実装する場合はもう少しoptionalな値を追加する必要がありますが、プレーンな実装だとこの程度でできると思います。

また、返却されるpayloadとしては、

 * UserID - ユーザーID
 * IsNewUser - 新規ユーザーフラグ
 * AccessToken - 認証後のAPIリクエストでバリデーションに使うJWT

などが要素として入ってきます。IDと新規ユーザーフラグは、ログやチュートリアル実装でよく使うので、そのまま返却してしまいます。ユーザー名などが必要な場合は、別途JWTから引き出すことも可能です。

=== init以外での非認証済みユーザーの除外

JWTをAccessTokenという値で返却する、と上述しましたが、その値は以下のような場面で利用します。

//list[middleware.go][認証用のミドルウェア]{
package middleware

import (
	"fmt"
	"net/http"
	"regexp"

	"github.com/pkg/errors"
	"github.com/timakin/sample_api/services/api/src/config"
	"github.com/timakin/sample_api/services/api/src/handler"
)

type Endpoint struct {
	Path   string
	Method string
}

var whiteList = []Endpoint{
	Endpoint{Path: `/ping`, Method: "GET"},
	Endpoint{Path: `/ping`, Method: "POST"},
	Endpoint{Path: `/api/stats`, Method: "GET"},
	Endpoint{Path: `/api/categories`, Method: "POST"},
	Endpoint{Path: `/api/topics`, Method: "POST"},
	Endpoint{Path: `/api/videos`, Method: "POST"},
}

func Authenticator(next http.Handler) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		// POST init endpointのみ認証は不要
		if ok, _ := regexp.MatchString(`^/[^/]+/init`, r.URL.Path); ok {
			if r.Method == "POST" {
				next.ServeHTTP(w, r)
				return
			}
		}

        // whiteListという配列にpathの
		for i := range whiteList {
			if ok, _ := regexp.MatchString(whiteList[i].Path, r.URL.Path); ok && r.Method == whiteList[i].Method {
				next.ServeHTTP(w, r)
				return
			}
		}

		at := handler.NewAuthenticator(config.AccessTokenSecret)
		token, err := at.ValidateFromRequest(r)
		if err != nil {
			err = errors.Wrap(err, fmt.Sprintf("Invalid JWT"))
			res := handler.NewErrorResponse(http.StatusUnauthorized, err.Error())
			handler.Redererer.JSON(w, res.Status, res)
			return
		}

		cr := r.WithContext(handler.NewContextWithJWT(r.Context(), token))
		next.ServeHTTP(w, cr)
	})
}
//}

このコードはmiddlewareの関数として定義されたもので、リクエストのたびに呼ばれますが、その役割は「init以外のリクエストではJWTの値を検証して、妥当なユーザーかどうかvalidateする」ということです。

認証処理というのは、デバイスの登録・更新に加えて、認証後に発行されたtokenを用いてAPIを許可されたユーザーだけが呼べるようにする、という一連の流れを指します。

Goではmiddlewareの関数を使ってこのように認証済みユーザー以外を弾くのが望ましいです。

== 必要なデータ定義

認証に必要になる構造体の定義としては、以下のようなものがあります。

//list[user.go][構造体定義]{
// User ... エンドユーザー
type User struct {
	ID        int64   `json:"id" datastore:"-" goon:"id"`
	CreatedAt int64   `json:"created_at"`
	UpdatedAt int64   `json:"updated_at"`
	Enabled   bool    `json:"enabled"`
	IsNew     bool    `json:"is_new" datastore:"-"`
	Device    *Device `json:"device" datastore:"-"`
}

// Device ... ユーザーの利用端末
type Device struct {
	ID           int64      `json:"id" datastore:"-" goon:"id"`
	UserID       int64      `json:"user_id"`
	DUID         string     `json:"duid"           validate:"duid"`
	OSName       string     `json:"os_name"        validate:"os_name"`
	OSTypeID     int        `json:"os_type_id"     validate:"os_type_id"`
	OSVersion    string     `json:"os_version"     validate:"os_version"`
	OSVersionNum int        `json:"os_version_num" validate:"os_version_num"`
	DeviceName   string     `json:"device_name"    validate:"device_name"`
	CreatedAt    int64      `json:"created_at"     validate:"created_at"`
	UpdatedAt    int64      `json:"updated_at"     validate:"updated_at"`
	DeviceApp    *DeviceApp `json:"device_app"     validate:"device_app" datastore:"-"`
}

// DeviceApp ... アプリ
type DeviceApp struct {
	ID            int64  `json:"id"                validate:"id" datastore:"-" goon:"id"`
	DeviceID      int64  `json:"device_id"         validate:"device_id"`
	BundleID      string `json:"bundle_id"         validate:"bundle_id"`
	AppVersion    string `json:"app_version"       validate:"app_version"`
	AppVersionNum int    `json:"app_version_num"   validate:"app_version_num"`
	CreatedAt     int64  `json:"created_at"        validate:"created_at"`
	UpdatedAt     int64  `json:"updated_at"        validate:"updated_at"`
}
//}

基本的なユーザー情報を表すUser、利用端末の情報を表したDevice、さらにそれにインストールされたDeviceAppという情報で構成します。

DeviceAppというのは、多くの場合Deviceに含めてしまっていい情報もありますが、たまにこのように一つの端末に複数の同一アプリがインストールされることがあります。

現に弊社ではプリインストールユーザーなどでユーザー情報の上書きを防ぐためにDeviceAppというところにアプリ情報を正規化して切り出すことで、そのバグを防いでいました。

== ビジネスロジック

実際のビジネスロジックはServiceという構造体が持っているというのを前述しましたが、以下がその実装です。

//list[service.go][認証ロジックの実装]{
type Service interface {
	Init(ctx context.Context, u *User) (*User, bool, string, error)
}

type service struct {
	repo Repository
}

// NewService creates a handling event service with necessary dependencies.
func NewService(repo Repository) Service {
	return &service{
		repo: repo,
	}
}

func (s service) Init(ctx context.Context, u *User) (*User, bool, string, error) {
	user, err := s.repo.GetUser(ctx, &User{
		Device: &Device{
			DUID: u.Device.DUID,
		},
	})
	if err != nil {
		if err == ErrResourceNotFound {
			err = nil
			err := datastore.RunInTransaction(ctx, func(tc context.Context) error {
				now := time.Now().Unix()
				u.Enabled = true
				u.CreatedAt = now
				u.UpdatedAt = now
				u.IsNew = true
				user, err = s.repo.UpsertUser(tc, u)
				if err != nil {
					return err
				}
				osVerNum, err := VerStrToInt(u.Device.OSVersion)
				if err != nil {
					return err
				}
				u.Device.UserID = u.ID
				u.Device.CreatedAt = now
				u.Device.UpdatedAt = now
				u.Device.OSTypeID = OSNameToTypeID[u.Device.OSName]
				u.Device.OSVersionNum = osVerNum
				d, err := s.repo.UpsertDevice(tc, u.Device)
				if err != nil {
					return err
				}
				user.Device = d

				appVerNum, err := VerStrToInt(u.Device.DeviceApp.AppVersion)
				if err != nil {
					return err
				}
				u.Device.DeviceApp.DeviceID = d.ID
				u.Device.DeviceApp.CreatedAt = now
				u.Device.DeviceApp.UpdatedAt = now
				u.Device.DeviceApp.AppVersionNum = appVerNum
				da, err := s.repo.UpsertDeviceApp(tc, u.Device.DeviceApp)
				if err != nil {
					return err
				}
				user.Device.DeviceApp = da

				return nil
			}, &datastore.TransactionOptions{
				XG: true,
			})
			if err != nil {
				return nil, false, "", err
			}
		} else {
			return nil, false, "", err
		}
	} else {
		user.IsNew = false
	}

	appVerNum, err := VerStrToInt(u.Device.DeviceApp.AppVersion)
	if err != nil {
		return nil, false, "", err
	}

	if user.Device.DeviceApp.AppVersionNum < appVerNum {
		user.Device.DeviceApp.AppVersion = u.Device.DeviceApp.AppVersion
		user.Device.DeviceApp.AppVersionNum = appVerNum
		user.Device.DeviceApp.UpdatedAt = time.Now().Unix()
		_, err := s.repo.UpsertDeviceApp(ctx, u.Device.DeviceApp)
		if err != nil {
			return nil, false, "", err
		}
	}

	// JWT生成
	token := user.NewAccessToken()
	at := handler.NewAuthenticator(config.AccessTokenSecret)
	sJWT, err := at.SignToken(token)
	if err != nil {
		return nil, false, "", err
	}

	return user, u.IsNew, sJWT, nil
}
//}

Initという必要の関数の中でUser, Device, DeviceAppを登録していきます。今回はGoogle Cloud PlatformのDatastoreをデータベースとして利用しているので、RunInTransaction という関数でトランザクションを張っています。

不恰好かもしれませんが、認証処理のように複数のテーブルを一挙に参照するような場面では、必ずトランザクションを張るべきです。

また、ここでは省略していますが、バージョン番号が古すぎる場合は強制アップデートフラグをつけるなどの処理もここで必要になってくるでしょう。

ユーザー情報を抜き取ったあとは、User構造体に用意したAccessToken化メソッドなどでデータをシリアライズします。

== リポジトリ

いわゆるデータアクセスするレイヤーの書き方はシンプルです。当該処理はサービスの処理の中から呼ばれます。 

//list[repository.go][データアクセスを担うリポジトリ層の実装]{
func (repo authRepository) UpsertUser(ctx context.Context, u *auth.User) (*auth.User, error) {
	g := goon.FromContext(ctx)
	if _, err := g.Put(u); err != nil {
		return nil, err
	}
	return u, nil
}
//}

Datastoreの場合、MySQLなどとは違い値にKeyという識別子が必要になってきますが、その発行処理をいちいち書いていたら面倒です。

ここではGoogle App Engineユーザー向けに最適化されたgoonというpackageを使いますが、他にも自前でラップするなどして、使いやすいクライアントを模索すべきです。

また、Google App Engine向けと書きましたが、App Engine以外からのDatastore利用の場合都合のいいパッケージがなかったので、筆者はtimakin/gostoというgoonと同様の処理をApp Engine以外からのCloud API呼び出しでできるようにしたものを作成しました。

== まとめ

上記のようにつらつらとコードとその補足を書いてきましたが、認証処理は通常のロジックとは違い、入り組んだデータ構造とトランザクション処理が必要になります。

書いたようなGoのcontextのような機構を使えばだいぶ楽にキャンセル処理をハンドリングできます。

JWTなどを容易に扱えるパッケージなども揃っているので、モバイル向けAPIとしてGoを採用し、認証処理を書く場面があれば、ぜひ今回の実装の一部だけでもご参考いただければと思います。



= Go Friday傑作選

== はじめに

株式会社ソウゾウのサーバサイドエンジニアの@tenntenn@<fn>{tenntenn_fn1}です。
本章では、筆者が所属する株式会社メルカリの子会社の株式会社ソウゾウにて、
毎週金曜日に行っている@<b>{Go Friday}という社内勉強会で出た話題を傑作選としてお届けします。

//footnote[tenntenn_fn1][@<href>{https://twitter.com/tenntenn}]

また、Go Fridayは社内勉強会にはめずらしく継続して開催されています。
多くの場合、社内勉強会が通常業務に忙殺されて長続きしないことは、読者のみなさんもおそらく経験されているでしょう。
しかし、Go Fridayは執筆時点（2017年10月）で1年以上毎週ほとんど休むことなく続いており、開催数は60回を超えています。
そこで本章ではまず、社内勉強会を長く続く意味のあるものにするためのコツを紹介します。

== 社内勉強会のススメ

=== Go Friday―ソウゾウ社内勉強会

ソウゾウでは、週に1回、夕方16時から@<kw>{Go Friday}という社内勉強会を開催しています。
Go Fridayという名前ですが、Go以外も扱っており、@<kw>{Google Cloud Platform,GCP}についても、
しばしば話題にあがります。
基本的には話題はなんでもよく、ソウゾウのサーバサイドエンジニアの関心事であれば問題ありません。
たとえば、新入社員の自己紹介を聞いて、その人のバックグラウンドを掘り下げたり、
社外勉強会やカンファレンスの発表練習を行ったりもします。

Go Fridayは長く続けるために、次のようなポリシーを設けています。

 * 発表資料の準備を頑張らない
 * スキップしない
 * ネタが無くても開催する

ここでは具体的にどのようにこのポリシーを元に社内勉強会を行っているのかを説明します。

=== 長く続けるためには"頑張らない"

社内勉強会が続かない大きな理由として、参加することが負担になるという点があるでしょう。
勉強会に参加するエンジニアは、開発業務を抱えています。
その中で、発表資料を作る必要があると負担となり、なかなか参加しづらいものです。

しかし、発表資料がなくとも勉強会を開くことは可能です。
社内であればプロジェクトのコードを直接見たり、
参考にしたブログやドキュメントのリンクを社内チャットやWikiに貼り付けるだけでも十分です。
必要があれば、ホワイトボードに図を書いて説明することもできるでしょう。

さらに発表資料を用意したからといって、議論が活発になるわけでもありません。
逆に資料を用意することで、議論の方向性を固定してしまう恐れがあります。
話したい話題と、参考にするリンクやコード片を貼り付けた社内Wikiのページを
共有するだけで議論は広がります。

=== エンジニアの雑談は宝

エンジニア、特に担当領域が同じサーバサイドエンジニアが集まれば、
たとえ雑談だとしても、それはきっとGoやGCPなど、技術的な話題になることが多いでしょう。
また、プロジェクトをまたぐと、同じサーバサイドエンジニアであっても普段はそこまで議論する機会は多くないでしょう。

ソウゾウにおいても「ありそうでなかったをソウゾウする」というミッションを掲げ、
@<b>{Move Fast}というバリューの下に新しいをアプリを次々に開発しています。
そのため、プロジェクト横断的な話題をする場としてもGo Fridayは機能しています。

このように、Go Fridayでは多様な内容を週1回というハイペースで扱っています。
次にGo Fridayで話題にあがった内容のうち、特に面白かったものについて解説していきます。

== switchを使おう

=== caseで式を使う

@<code>{switch}は定数で処理を分岐させるためによく使われる構文です。
Goの@<code>{switch}は、@<list>{tenntenn_list1}のように、@<code>{case}に式を取れるため、
煩雑な@<code>{if-else}の連鎖をスッキリと書くことができます。

//list[tenntenn_list1][@<code>{case}で式を取る例]{
switch {
    case a == 1:
        fmt.Println("a is 1")
    case a == 2:
        fmt.Println("a is 2")
}
//}

また、Goの@<code>{switch-case}は明示的に@<code>{break}を指定しなくても、
@<code>{case}の末尾で@<code>{switch}を抜けるようになっています。
そのため、毎@<code>{case}ごとに@<code>{break}を書く必要がなく、
コード量も少なくて済みます。

=== 同じcaseをコンパイルで弾く

ここでは@<code>{switch}を用いることで、よくあるバグをコンパイル時に弾く方法について説明します。

ステータスなどを管理する際に、たとえば@<list>{tenntenn_list2}のように定数を用いることが多いです。
ここでは、@<code>{type}で新しい型@<code>{Status}を作り、その定数として@<code>{StatusA}と@<code>{StatusB}を定義しています。

//list[tenntenn_list2][ステータスを定数で定義した例]{
type Status int64
const (
	StatusA Status = 0
	StatusB Status = 1
)
//}

これらの2つの定数は、値が意味を持っているため、
@<code>{iota}を使わずに、@<code>{0}や@<code>{1}のような具体的な値を直接指定しています。
@<code>{iota}を使う場合は、値として意味があるわけではなく、それぞれの定数の値が違っていれば良い場合に限られます。
@<code>{iota}で定義している定数の間に、機能改修などでうっかり新しい定数を追加すると、それ以降の定数の値が変わってしまうからです。
また、その定数の値をデータベースに保存する必要がある場合は、なおさら@<code>{iota}は使うべきではありません。

さて、@<code>{switch}の話に戻します。
前述した2つの定数を使って、@<list>{tenntenn_list3}のような@<code>{switch}を書いたとします。
@<code>{f}関数は引数で渡された@<code>{Status}によって、表示される文字を変える関数です。

//list[tenntenn_list3][定数をswitchで分岐する例]{
func f(s Status) {
	switch s {
	case StatusA:
		fmt.Println("ステータスA")
	case StatusB:
		fmt.Println("ステータスB")
	}
}
//}

ここでステータスを1つ増やす要件が入ったとします。
@<code>{StatusB}の定義をコピーし、定数@<code>{StatusC}を増やし、
そして@<code>{case}を1つ追加します。
そうすると@<list>{tenntenn_list4}のようになります。

//list[tenntenn_list4][StatusCを増やした例]{
type Status int64
const (
	StatusA Status = 0
	StatusB Status = 1
	StatusC Status = 1
)

func f(s Status) {
	switch s {
	case StatusA:
		fmt.Println("ステータスA")
	case StatusB:
		fmt.Println("ステータスB")
	case StatusC:
		fmt.Println("ステータスC")
	}
}
//}

いかかでしょう。うまく動きそうでしょうか。
実はこのままだとうまく動きません。
@<code>{StatusC}の定義が@<code>{StatusC Status = 1}となっているからです。
この定義だと@<code>{StatusB}と同じ値になってしまいます。
本来であれば、@<code>{StatusC Status = 2}とするべきです。

さて、このままだと実行時にエラーが発生してしまいそうです。
実際に実行してみようとすると、次のようなエラーが発生します。

//cmd{
$ go run main.go
main.go:21:2: duplicate case StatusC (value 1) in switch
    previous case at main.go:11:19
//}

よく見るとこれは実行時エラーではなく、コンパイルエラーです。
エラーメッセージによると、@<code>{case}で使われている値が重複しているというものです。
実際にGo Playground@<fn>{tenntenn_fn2}で動してみると、コンパイルエラーが発生することが分かります。

//footnote[tenntenn_fn2][@<href>{https://play.golang.org/p/RafJs_O9y8}]

このように、Goの@<code>{switch}では、@<code>{case}の値が重複していると、
コンパイルエラーになります。
新しく定数を追加する際に、前述のようなミスはよく発生します。
そうした際に、コンパイルエラーになると予め間違いに気づけるため非常に助かります。
そういった意味でも積極的に@<code>{switch}を使うと良いです。

== サブテストとテーブル駆動テスト

=== サブテスト

Go1.7でサブテストという機能が入りました。
この機能は@<list>{tenntenn_list5}のように、1つのテスト関数内で、サブテストを複数実行できる機能です。

//list[tenntenn_list5][サブテストの例]{
func TestAB(t *testing.T) {
    t.Run("A", func(t *testing.T) { t.Error("error") })
    t.Run("B", func(t *testing.T) { t.Error("error") })
}
//}

サブテストは、@<code>{*testing.T}の@<code>{Run}メソッドに対して、
サブテスト名と関数を引数で指定して実行します。
第2引数の関数は通常のテスト関数と同様に、@<code>{*testing.T}を引数に取る関数です。

サブテストは、何も指定しないと、@<code>{Run}メソッドで指定したテスト関数をすべて実行します。
しかし、次のようにテスト名とサブテスト名を指定して実行することができます。

//cmd{
go test -v sample -run TestAB/A
=== RUN   TestAB
=== RUN   TestAB/A
--- FAIL: TestAB (0.00s)
   --- FAIL: TestAB/A (0.00s)
       sample_test.go:10: error
FAIL
exit status 1
FAIL    sample    0.007s
//}

一見、何のためにある機能か分かりづらいですが、
次に説明する@<kw>{テーブル駆動テスト}と組み合わせるとその効果を発揮します。

=== テーブル駆動テスト

サブテストとテーブル駆動テストの組み合わせを述べる前に、
まずはテーブル駆動テストについて説明します。

テーブル駆動テストとは、@<list>{tenntenn_list6}のように、
テストケースを構造体のスライスで並べ、それらのテストケースに対して、
@<code>{for}で繰り返して各テストケースをテストするというものです。

//list[tenntenn_list6][テーブル駆動テストの例]{
var flagtests = []struct {
    in  string
    out string
}{
    {"%a", "[%a]"}, {"%-a", "[%-a]"}, {"%+a", "[%+a]"},
    {"%#a", "[%#a]"}, {"% a", "[% a]"},
}
func TestFlagParser(t *testing.T) {
    var flagprinter flagPrinter
    for _, tt := range flagtests {
        s := Sprintf(tt.in, &flagprinter)
        if s != tt.out {
            t.Errorf("Sprintf(%q, &flagprinter) => %q, want %q", tt.in, s, tt.out)
        }
    }
}
//}

テーブル駆動テスト利点は、1度テストを書くと、簡単にテストケースを追加することができる点です。
テーブル駆動テストによって大量のテストケースを簡単に書くことができます。
こうすることで、網羅的なテストが書きやすく、コードの品質の向上につながります。

また、テーブル駆動テストを書く際に、さまざまなケースをについて考慮する必要があり、
テストをしやすいインタフェース = ユーザが使いやすいコードとなります。
ここでいう"ユーザ"とは、テスト対象のコードを使う人であり、ライブラリのユーザやチームメンバー、
もしかしたら未来の自分かもしれません。

=== サブテストとテーブル駆動テスト

テーブル駆動テストでテストを書いていくと、テストケースが大量にできます。
たとえば、数十から数百からのテストケースは容易に考えられます。

テストはテストが落ちた時に、どこで落ちたのか、
なぜ落ちたのかを分かりやすく知れることが大切です。
テーブル駆動テストの場合、@<code>{for}で回しているため、
どのケースでテストが落ちても、@<code>{go test}は常に同じ行数を指します。
その際、テストケースが大量にあると、どのテストケースでテストが
落ちたのか分かりづらくなります。

テストケースに名前を付けて、テストが落ちた際のエラーメッセージに
@<list>{tenntenn_list7}のようにテストケース名を出力すればわかりやすくなります。

//list[tenntenn_list7][テストケースに名前を付けた例]{
func TestIsOdd(t *testing.T) {
    cases := []*struct {
        name     string
        input    int
        expected bool
    }{
        {name: "+odd", input: 5, expected: true},
        {name: "+even", input: 6, expected: false},
        {name: "-odd", input: -5, expected: true},
        {name: "-even", input: -6, expected: false},
        {name: "zero", input: 0, expected: false},
    }
    for _, c := range cases {
        if actual := IsOdd(c.input); c.expected != actual {
            t.Errorf("%s: want IsOdd(%d) = %v, got %v",
                c.name, c.input, c.expected, actual)
        }
    }
}
//}

しかし、まだ問題が残ります。
たとえば、300件のテストケースがあり200番目のテストが落ちた場合を考えます。
200番目のテストが通るようにコードを修正し、再度テストを走らせたいと考えるでしょう。
できれば該当の部分だけを実行したいですが、@<list>{tenntenn_list7}のコードでは、
再度すべてのテストケースを走らせる必要があります。

このような場合に、サブテストを使うとサブテスト名を指定して実行することができます。
テストの修正も簡単で、@<list>{tenntenn_list8}のように、
テストを行っている@<code>{if}を@<code>{t.Run}で渡した関数内中に移せば良いのです。

//list[tenntenn_list8][テストケースに名前を付けた例]{
func TestIsOdd(t *testing.T) {
    cases := []*struct {
        name     string
        input    int
        expected bool
    }{
        {name: "+odd", input: 5, expected: true},
        {name: "+even", input: 6, expected: false},
        {name: "-odd", input: -5, expected: true},
        {name: "-even", input: -6, expected: false},
        {name: "zero", input: 0, expected: false},
    }
    for _, c := range cases {
        t.Run(c.name, func(t *testing.T) {
            if actual := IsOdd(c.input); c.expected != actual {
                t.Errorf("want IsOdd(%d) = %v, got %v",
                    c.input, c.expected, actual)
            }
        })
    }
}
//}


こうするだけ、@<code>{go test}の@<code>{-run}オプションを用いて、
次のように実行することができます。
この場合は、@<code>{-odd}というテストケースのみを実行しています。

//cmd{
go test -v sample -run TestIsOdd/-odd
=== RUN   TestIsOdd/-odd
--- PASS: TestIsOdd/-odd (0.00s)
PASS
ok      command-line-arguments  0.006s
//}

このように、サブテストとテスト駆動テストを組み合わせると、
テストケースを大量に作ることができ、さらにテストが落ちた際の
メッセージや再実行の方法も分かりやすくなります。

== Go1.9の新機能テストヘルパー

=== テストヘルパーの作り方

テストを書く際に、よく使う処理を関数にまとめたくなります。
そうした関数をここではテストヘルパーと呼ぶことにします。
そして、テストヘルパーは次の2つを満たすように作るとよいです。

 * 引数で@<code>{*testing.T}型の値を受け取る
 * エラーは返さずに@<code>{Fatal}にする

たとえば、一時ファイルを作成するテストヘルパーは@<list>{tenntenn_list9}のようになります。

//list[tenntenn_list9][一時ファイルを作るテストヘルパー]{
func testTempFile(t *testing.T) string {
    tf := ioutil.TempFile("", "test")
    if err != nil {
        t.Fatal("err %s", err)
    }
    tf.Close()
    return tf.Name()
}
//}

このテストヘルパーでは、一時ファイルを作成する際に、@<code>{ioutil.TempFile}を使用しています。
@<code>{ioutil.TempFile}は戻り値としてエラーを返す可能性があります。
通常であれば、適切にエラー処理し、必要があれば戻り値として呼び出し元に返します。
しかし、テストヘルパーでエラーを返してしまうと、各テスト関数内で毎回エラー処理をする必要があります。
また、テストヘルパー内で発生するエラーは、ほとんどが回復不可能なエラーです。
そのため、エラーを返すのではなく、@<code>{t.Fatal}メソッドでテストを落としてやる方がよいです。
そのためにも、テストヘルパーには引数として@<code>{*testing.T}型の値を取るようにするべきです。

=== エラーメッセージを有用なものにする

テストヘルパーを作った場合に1つ困ることが生じます。
それはテストヘルパー内で落ちた際に、エラーメッセージが分かりづらいということです。
@<code>{go test}は、テストが落ちた際にエラーメッセージとして、
次のようにテストが落ちた行を表示します。
これはテスト関数中で@<code>{t.Fatal}メソッドや@<code>{t.Error}メソッドを呼び出した行です。

//cmd{
$go test -v hoge_test.go
=== RUN   Test
--- FAIL: Test (0.00s)
        hoge_test.go:6: エラー
FAIL
exit status 1
FAIL    command-line-arguments  0.011s
//}

テストヘルパー内で@<code>{t.Fatal}メソッドや@<code>{t.Error}メソッドを呼び出した場合、
エラーメッセージとしてテストヘルパー内の行が表示されます。
そうなるとテストヘルパーはさまざまなテスト関数から呼び出されるため、
エラーメッセージが非常に分かりづらくなります。

そこで、Go1.9から@<code>{Helper}メソッドが@<code>{*testing.T}型に導入されました。
このメソッドは、@<list>{tenntenn_list10}のように、テストヘルパーの先頭で呼び出します。
そうすることで、テストヘルパー内でテストが落ちた際に、
エラーメッセージに表示される行がテストヘルパーを呼び出した行に変更されます。

//list[tenntenn_list10][Helperメソッドを使った例]{
func testTempFile(t *testing.T) string {
    t.Helper()
    tf := ioutil.TempFile("", "test")
    if err != nil {
        t.Fatal("err %s", err)
    }
    tf.Close()
    return tf.Name()
}
//}

このように、テストヘルパーをうまく使ってテストを簡略化することができます。
そして、テストヘルパーは@<code>{t.Helper}メソッドを用いることで、
有用なエラーメッセージを表示することが可能になります。

== 一時的なエラーのハンドリング

=== エラーの種類によるハンドリング

Goのエラーハンドリングは、主にエラーがあったかどうかを@<code>{nil}と
比較することで判定することが多いです。
しかし、エラーの種類でハンドリングする必要がある場合もあります。

そのような場合は、エラーを値で比較するのではなく、インタフェースを用いると良いです。
インタフェースを用いることで、具体的な値や型に依存することなく、
エラーの種類でハンドリングすることができるようになります。

たとえば、とあるエラーが一時的なものかどうかを判定したいとします。
そして、それを判定する関数として@<code>{IsTemporary}関数を@<list>{tenntenn_list11}のように定義します。

//list[tenntenn_list11][インタフェースを用いた判定関数]{
type temporary interface {
    Temporary() bool
}

// IsTemporary は一時エラーの場合はtrueを返す
func IsTemporary(err error) bool {
    te, ok := err.(temporary)
    return ok && te.Temporary()
}
//}

@<code>{IsTemporary}関数は、引数に@<code>{error}を取り、
それが一時エラーかどうかを返します。
一時エラーかどうかの判定には、次の2つを用いています。

 * @<code>{temporary}インタフェースを実装しているか
 * 実装していた場合@<code>{Temporary}メソッドが@<code>{true}を返すか

型アサーションで@<code>{err}を@<code>{temporary}型にキャストし、
キャストができた場合はさらに@<code>{Temporary}メソッドを呼び出しています。

さて、一時エラーを判定する関数を作成しましたが、利用側はどうすればよいでしょうか。
まずは@<code>{temporary}インタフェースを実装する型を作る必要があります。
@<code>{IsTemporary}関数は、詳しい実装には依存しないため、
なんでも良いのですが、たとえば@<list>{tenntenn_list12}のような実装を考えましょう。

//list[tenntenn_list12][一時エラーの作成]{
type tmperr struct {
    original error
}

func (err *tmperr) Error() string {
    return err.original.Error()
}

func (err *tmperr) Temporary() bool {
    return true
}

func Temporary(err error) error {
    return &tmperr{original: err}
}
//}

@<code>{tmperr}型は既存のエラーをラップし、一時エラーとして振る舞わせる型です。
@<code>{original}フィールドとして、基となるエラーを保持します。

@<code>{tmperr}型はエクスポートされずに、@<code>{Temporary}関数によって生成されます。
このとき、@<code>{Temporary}関数の戻り値が@<code>{error}インタフェースであることに注意してください。
こうすることで、@<code>{Temporary}関数の利用者は、具体的な実装について知る必要がなくなります。

さて、一時エラーを作るところまで説明しました。
次に一時エラーをハンドリングする方法を説明しましょう。

一時エラーのハンドリングは、@<code>{IsTemporary}関数を使い、
@<list>{tenntenn_list13}のように行います。
@<code>{switch}で、エラーが@<code>{nil}かどうか、
@<code>{nil}じゃない場合、一時エラーかどうかを判定しています。
こういったエラーハンドリングの場合も@<code>{switch}を使うと見通しがよいですね。

//list[tenntenn_list13][一時エラーのハンドリング]{
func f() error {
    return Temporary(errors.New("失敗しました"))
}

func main() {
    err := f()
    switch {
    case err == nil:
        fmt.Println("エラーなし")
    case IsTemporary(err):
        fmt.Println("一時エラー", err)
    default:
        fmt.Println("エラー", err)
    }
}
//}

このようにインタフェースを用いることで、利用者は具体的な実装に依存すること無く、
エラーの種類によってエラーハンドリングを行うことができます。

=== エラーはerrorのまま扱う

@<code>{IsTemporary}の例でみたように、エラーを引数や戻り値で扱う場合、
@<code>{error}型で扱うことが多いでしょう。
しかし、これはエラーに限ったことではありません。

インタフェースを実装する型を作った場合、
その実装はできる限りパッケージ内に隠蔽すると良いです。
そうすることで、利用者からは具体的な実装を知ることなく、
内部の実装が変わったとしても利用者は自身のコードを変更する必要がありません。

このように、できる限りインタフェースをパッケージ間の境界、
つまりその名の通りインタフェースに置くことでパッケージ間の
依存の強さを極力抑えることができます。

=== 一時的なエラーとエラーログ

さて、ここで紹介した一時エラーを判定するしくみですが、
実際に使う場面はあるのでしょうか。
筆者がサーバサイドエンジニアとして関わっているメルカリ カウル@<fn>{tenntenn_fn3}では、
実際にこの方法で一時エラーかどうかをハンドリングしている箇所が存在します。

//footnote[tenntenn_fn3][@<href>{https://mercarikauru.com/}]

メルカリ カウルでは、サーバサイドで実行時エラーが発生した場合、
それをエラーログとして出力しています。
また、本番環境でエラーログが出力されると、Slackのモニタリングチャンネルに通知が飛ぶようになっています。

エラーが起きた場合にSlackに通知されるのは、いち早くエラーに気づけるため、非常に便利です。
しかし、一方で通知が多すぎると逆に重要なエラーがわかりづらくなります。

そこで一時エラーを用いることにより、一時エラーの場合はエラーではなく
ワーニングとしてログに出力するようにしました。
こうすることで、重要なエラーだけがSlackに通知されるようになりました。

この方法は特にGoogle App Engineのタスクキューを使って処理をする場合に有効です。
タスクキューでは、エラーとしてHTTPのレスポンスを返すと、リトライを行ってくれます。
そのため、エラーとして処理は返したいけど、そのうち回復するエラーだからSlackには通知してほしくない、といった場合に一時エラーを利用することは有効です。

== おわりに

本章では、筆者が開催しているGo Fridayとそこで扱った特に面白い話題についてご紹介しました。
社内勉強会ではたとえ準備を頑張らなくとも、ここで紹介したレベルの議論をすることができます。

ぜひ読者のみなさんの所属する企業や学校でも、続けることを念頭において、
気軽に参加できるエンジニアの議論の場としての勉強会を開いてみてはいかがでしょうか。

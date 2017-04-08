<?php
    // DB設定ファイルを読み込む
    // ※設置場所へのパス(../)は各自の環境に合わせてください。
    require_once "../../iot.inc"; 

    // DB接続(UTF8)
    // ※ホスト(localhost)は必要に応じて変更してください。
    $mysqli = @new mysqli("localhost", DB_ID, DB_PASS, DB_NAME);     
    if ($mysqli->connect_error) {
      die("DB接続エラー");
    }   
    $mysqli->set_charset('utf8');        
    
    // メッセージ用    
    $msg = 'NG';
    
    // DB用変数
    $db_type = "GET";
    if ($_GET['value'] == ""){ 
      $db_value = 0; 
    }else{
      $db_value = $_GET['value'];
    }
    
    if ($_GET['string'] == ""){ 
      $db_string = ""; 
    }else{
      $db_string = $_GET['string'];
    }
       
    // SQL               
    $sql  = "INSERT INTO t_example (type,value,string,reg_date) VALUES(?,?,?,now())";
    
    // トランザクションの開始
    if ($mysqli->autocommit(FALSE)){    
        // プリペアドステートメントの作成                                   
        if ($stmt = $mysqli->prepare($sql)) {            
            // マーカにパラメータをバインド(s= string d= double i=integer)
            if ($stmt->bind_param("sis",                   
                                  $db_type,   // type
                                  $db_value,  // value
                                  $db_string  // string                                          
                                 )){
                // 実行
                if ($stmt->execute()){
                    $stmt->close();                                 
                    // コミット                           
                    if ($mysqli->commit()) {
                        $msg = 'OK'; // DB登録成功
                    // ロールバック    
                    }else{
                        $mysqli->rollback();
                    }                                  
                }
             }
        }
    }          
    $mysqli->close();           
?>
<?php 
  // ヘッダの出力
  // ※以下の2行は必ず出力して下さい。
  header("Content-Type: text/html; charset=utf-8");
  header("Content-Length: " . strlen($msg));
  
  echo $msg;
  
  // ファイル末尾に空白などがあるとその文字列も出力されるので注意して下さい。
?>
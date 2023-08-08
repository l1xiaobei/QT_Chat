//void chat::on_imageButton_clicked()
//{
//    QString fileName = QFileDialog::getOpenFileName(this, tr("选择图片"), "*", tr("Images(*.png, *.jpg, *.jpeg, *.bmp, *.gif, *.webp, *.apng, *.sharpp)"));
//    if(fileName.isEmpty()!=true)//得先确保选择图片了
//    {
//        QImage image(fileName); //创建图片对象
//        QByteArray imgBy;
//        QBuffer imgBuf(&imgBy);
//        image.save(&imgBuf, "png");//将图片数据写入到imgBy字节数组中
//        //利用QDataStream封装信息传送图片
//        QByteArray infoBlock;
//        QDataStream out(&infoBlock, QIODevice::WriteOnly);   //将out与infoBlock关联起来
//        out.setVersion(QDataStream::Qt_5_14);       //设置QDataStream版本
//        int typeMsg = MsgType::imageMsg;        //设置发送的消息类型是图片类型
//        QDateTime dateTime= QDateTime::currentDateTime();//获取系统当前的时间//这个时间差不多是接收到的时间而不是发送的时间
//        QString timeStr = dateTime.toString("yyyy-MM-dd hh:mm:ss");//格式化时间
//        /*本地显示*/
//        ui->chatText->append(timeStr + "  " + ui->nameLine->text() + ": " + "\n" + ui->sendLine->text());
//        QString imgBase64 = imgBy.toBase64();
//        qDebug()<<imgBase64 ;
//        QString htmlPath = "<img src=\"data:image/png;base64," + imgBase64 + "width=\"220\" height=\"96\"" "\">";
//        ui->chatText->insertHtml(htmlPath);
//        /*发送实现*/
//        int totalSize = imgBy.size();
//        qDebug() << totalSize;
//        int sendSize = 0;
//        QByteArray userInfo;
//        userInfo.append(timeStr + "  " + ui->nameLine->text() + ": " + "\n" + ui->sendLine->text());//发送用户的信息
//        const int CHUNK_SIZE = 1024;    //每帧发送的字节大小
//        qDebug()<<userInfo;
//        int chunkSize, remainingSize,blockIndex=1;
//        while(sendSize < totalSize)
//        {
//            remainingSize = totalSize - sendSize;
//            chunkSize = qMin(remainingSize, CHUNK_SIZE); //每帧发送的字节大小,如果不够1024，剩多少发多少
//            //chunk = imgBy.left(chunkSize);  //从原始数据中提取一个块 //不对！这样提取没把原来的删掉，所以每次提取的数据都是一样的
//            QByteArray chunk = imgBy.mid(sendSize, chunkSize);//这样从原始数据中提取块就合理了
//            out<<blockIndex<<typeMsg<<totalSize<<chunk.length()<<userInfo<<chunk;      //将消息全部封装进infoBlock
//            socket->write(infoBlock);            //发送块数据
//            QThread::usleep(3);
//            sendSize += chunkSize;                           //更新已发送数据的大小

//            //qDebug()<<sendSize<<"|||"<<remainingSize<<"||| chunkLength"<<chunk.length();
//            qDebug()<<blockIndex<<"|||"<<typeMsg<<totalSize<<chunk.length()<<userInfo<<"~~~"<<endl;
//            blockIndex+=1;//包索引
//            //qDebug() << sendSize << "--->" << double(100.0*sendSize/totalSize) << "%";
//        }
////        qDebug()<<"---------------------------------------"<<endl<<imgBy;
//    }

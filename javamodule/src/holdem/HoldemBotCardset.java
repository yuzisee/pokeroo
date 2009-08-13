/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package holdem;


import com.biotools.meerkat.Hand;
import com.biotools.meerkat.Card;

/**
 *
 * @author acer
 */
public class HoldemBotCardset {

    private long ptr_HoldemCardset;

    native void CreateNewCardset();
    native void AppendCard(char cardValue,char cardSuit);
    native void DeleteCardset();


    private void AppendCard(Card c)
    {
        char cardsuit;
        switch(c.getSuit())
        {
            case Card.SPADES:
                cardsuit = 's';
                break;
            case Card.HEARTS:
                cardsuit = 'h';
                break;
            case Card.CLUBS:
                cardsuit = 'c';
                break;
            case Card.DIAMONDS:
                cardsuit = 'd';
                break;
            default:
                throw new RuntimeException("c.getSuit(): " + c.getSuit() + " fails to match known suit");
        }

        char cardrank;
        switch(c.getRank())
        {
            case Card.ACE:
                cardrank = 'A';
                break;
            case Card.TWO:
                cardrank = '2';
                break;
            case Card.THREE:
                cardrank = '3';
                break;
            case Card.FOUR:
                cardrank = '4';
                break;
            case Card.FIVE:
                cardrank = '5';
                break;
            case Card.SIX:
                cardrank = '6';
                break;
            case Card.SEVEN:
                cardrank = '7';
                break;
            case Card.EIGHT:
                cardrank = '8';
                break;
            case Card.NINE:
                cardrank = '9';
                break;
            case Card.TEN:
                cardrank = 't';
                break;
            case Card.JACK:
                cardrank = 'J';
                break;
            case Card.QUEEN:
                cardrank = 'Q';
                break;
            case Card.KING:
                cardrank = 'K';
                break;
            default:
                throw new RuntimeException("c.getRank(): " + c.getRank() + " fails to match known rank");
        }

        AppendCard(cardrank,cardsuit);
    }

    public HoldemBotCardset(Hand cardset)
    {
        CreateNewCardset();
        for( int cardidx = 1; cardidx < cardset.size(); ++cardidx )
        {
            AppendCard(cardset.getCard(cardidx));
        }
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            DeleteCardset();        // close open files
        } finally {
            super.finalize();
        }
    }

}
